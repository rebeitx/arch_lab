from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import time
import signal
import sys
import RPi.GPIO as GPIO

# === КОНФИГУРАЦИЯ ===
HOST = "0.0.0.0"
PORT = 8080
MAX_TIME_MS = 10000
# ===================

# Пины моторов в нумерации BCM
PINS = {
    "L_FWD": 12,  # Левый вперёд
    "L_BWD": 13,  # Левый назад
    "R_FWD": 20,  # Правый вперёд
    "R_BWD": 21   # Правый назад
}

# Класс управления моторами робота через GPIO
class Robot:
    def __init__(self):
        # Инициализация GPIO и установка всех пинов в LOW
        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)
        for pin in PINS.values():
            GPIO.setup(pin, GPIO.OUT)
            GPIO.output(pin, GPIO.LOW)
        print("🤖 Робот инициализирован")

    def _set_motors(self, l_fwd, l_bwd, r_fwd, r_bwd):
        # Устанавливаем состояние всех четырёх пинов моторов
        GPIO.output(PINS["L_FWD"], l_fwd)
        GPIO.output(PINS["L_BWD"], l_bwd)
        GPIO.output(PINS["R_FWD"], r_fwd)
        GPIO.output(PINS["R_BWD"], r_bwd)

    def stop(self):
        print("⏹️ Остановка")
        self._set_motors(False, False, False, False)

    def forward(self, time_ms):
        print(f"⬆️ Вперёд {time_ms} мс")
        self._set_motors(True, False, True, False)
        time.sleep(time_ms / 1000.0)
        self.stop()

    def left(self, time_ms):
        print(f"⬅️ Влево {time_ms} мс")
        # Левый назад, правый вперёд — разворот на месте влево
        self._set_motors(False, True, True, False)
        time.sleep(time_ms / 1000.0)
        self.stop()

    def right(self, time_ms):
        print(f"➡️ Вправо {time_ms} мс")
        # Левый вперёд, правый назад — разворот на месте вправо
        self._set_motors(True, False, False, True)
        time.sleep(time_ms / 1000.0)
        self.stop()

    def cleanup(self):
        self.stop()
        GPIO.cleanup()


# Создаём экземпляр робота
robot = Robot()


# Обработчик сигналов для безопасного завершения
def graceful_exit(sig, frame):
    print("\nОстановка...")
    robot.cleanup()
    sys.exit(0)

signal.signal(signal.SIGINT, graceful_exit)
signal.signal(signal.SIGTERM, graceful_exit)


# Обработчик HTTP-запросов
class Handler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        pass  # Отключаем стандартные логи

    def do_POST(self):
        # Принимаем POST /command с JSON {"action": "...", "time": ...}
        if self.path == "/command":
            length = int(self.headers.get("Content-Length", 0))
            data = json.loads(self.rfile.read(length))

            command = data.get("command", "stop")
            time_ms = max(0, min(int(data.get("time", 0)), MAX_TIME_MS))

            print(f"[CMD] {command} {time_ms} мс")

            # Выполняем команду
            if command == "forward":
                robot.forward(time_ms)
            elif command == "left":
                robot.left(time_ms)
            elif command == "right":
                robot.right(time_ms)
            else:
                robot.stop()

            self.send_response(200)
            self.end_headers()


# Запуск сервера
print(f"[OK] Сервер запущен на {HOST}:{PORT}")
HTTPServer((HOST, PORT), Handler).serve_forever()
