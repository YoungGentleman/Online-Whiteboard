# Whiteboard

Совместная онлайн-доска для рисования в реальном времени. Несколько пользователей могут рисовать одновременно — один запускает приложение как хост (сервер), остальные подключаются к нему по IP и порту. Написано на C++17 с использованием Qt6.

## Сборка на Ubuntu 22.04+

### 1. Установить зависимости

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build git \
    libgl1-mesa-dev libxcb-cursor0 \
    qt6-base-dev qt6-tools-dev libqt6network6-dev
```

### 2. Клонировать репозиторий

```bash
git clone https://github.com/YoungGentleman/Online-Whiteboard.git
cd whiteboard
```

### 3. Собрать

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Или для Debug-версии:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

### 4. Запустить

```bash
./build/whiteboard
```
