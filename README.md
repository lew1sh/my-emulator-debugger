
# My Emulator Debugger

Учебный проект: простой эмулятор "виртуальной машины" на C++ + Debug Adapter (DAP) + отладчик для Visual Studio Code.

С помощью этого проекта можно:

- запускать простые программы в текстовом формате (`program.emu`);
- ставить точки останова (breakpoints) в файле `program.emu`;
- выполнять программу пошагово (Step Over);
- смотреть текущие значения регистров `pc` и `acc` в панели VARIABLES VS Code.

---

## 1. Что внутри проекта

Структура репозитория (пример):

```text
my-vm-debugger/
  CMakeLists.txt         # корневой CMake
  emulator/              # C++ эмулятор
    include/emulator.h
    src/emulator.cpp
  adapter/               # C++ debug adapter (реализация DAP)
    src/debug_adapter.cpp
  vscode-extension/      # расширение VS Code
    package.json
    tsconfig.json
    src/extension.ts
  build/                 # каталог сборки (создаётся пользователем)
````

### Эмулятор

Эмулятор хранит:

* `pc` — счётчик команд (номер текущей инструкции);
* `acc` — аккумулятор (целое число);
* `state` — состояние: `Stopped`, `Running`, `Exited`;
* `program` — вектор инструкций (`ADD`, `SUB`, `JMP`, `HLT`);
* `breakpoints` — множества адресов команд, где нужно остановиться.

Поддерживаемые инструкции:

* `ADD N` — `acc += N; pc++`
* `SUB N` — `acc -= N; pc++`
* `JMP N` — `pc = N`
* `HLT 0` — остановка программы (`state = Exited`)

Файл программы (`program.emu`) — одна инструкция на строку:

```text
ADD 1
ADD 2
SUB 1
HLT 0
```

### Debug Adapter

Бинарник `debug_adapter` (C++):

* запускается VS Code как отдельный процесс;
* читает JSON-сообщения DAP из `stdin`;
* вызывает методы эмулятора (`loadProgram`, `run`, `step`, управление брейкпоинтами);
* отправляет ответы и события (initialize, stopped, terminated, output, stackTrace, scopes, variables и т.д.) в `stdout` в формате DAP.

### Расширение VS Code

Папка `vscode-extension/`:

* `package.json` — манифест расширения:

  * регистрирует новый тип отладчика `"my-emulator"`;
  * включает поддержку breakpoints для языка `plaintext`;
  * добавляет шаблон конфигурации запуска.
* `tsconfig.json` — сборка TypeScript → JS в `out/`.
* `src/extension.ts`:

  * реализует `activate()`;
  * регистрирует фабрику `DebugAdapterDescriptorFactory`;
  * указывает путь к бинарнику `debug_adapter`.

---

## 2. Требования

Для сборки и запуска нужны:

* **C++ компилятор** с поддержкой C++17 (g++, clang, MSVC);
* **CMake** версии 3.15+;
* **Node.js + npm**;
* **Visual Studio Code**.

---

## 3. Сборка C++ части (эмулятор + debug adapter)

В корне проекта:

```bash
cd my-vm-debugger

mkdir build
cd build

cmake ..
cmake --build .
```

После успешной сборки должны появиться, например:

* `build/emulator/libemulator.a` (или аналогичная библиотека),
* `build/adapter/debug_adapter` — **главный бинарник debug adapter**.

Запомните путь к `debug_adapter`, он понадобится расширению VS Code.

---

## 4. Настройка расширения VS Code

### 4.1. Установка зависимостей и сборка TypeScript

```bash
cd vscode-extension
npm install
npm run compile    # скомпилирует src/extension.ts в out/extension.js
```

### 4.2. Проверить путь к `debug_adapter`

В файле `vscode-extension/src/extension.ts` есть строка:

```ts
const command = "/Users/tim/Desktop/my-vm-debugger/build/adapter/debug_adapter";
```

Её нужно заменить на **реальный путь** до `debug_adapter` на вашей машине, например:

```ts
const command = "/ABSOLUTE/PATH/TO/my-vm-debugger/build/adapter/debug_adapter";
```

После изменения — снова:

```bash
npm run compile
```

---

## 5. Как запускать отладчик в VS Code

### 5.1. Запуск расширения в режиме разработки

1. Откройте папку `vscode-extension/` в обычном VS Code.
2. Убедитесь, что файл `out/extension.js` существует (после `npm run compile`).
3. Нажмите `F5` — запустится второе окно **Extension Development Host**.
   Это отдельный VS Code, в котором расширение уже активно.

Дальнейшие шаги выполняются **в этом втором окне**.

### 5.2. Подготовить файл программы

В окне Extension Development Host:

1. Откройте папку с проектом / или любую папку, где будет лежать программа.

2. Создайте файл `program.emu`, например:

   ```text
   ADD 1
   ADD 2
   SUB 1
   HLT 0
   ```

3. Убедитесь, что для файла выбран язык **Plain Text** (в правом нижнем углу VS Code).

### 5.3. Создать `launch.json`

Откройте командную палитру (Ctrl+Shift+P / Cmd+Shift+P):

* выберите **“Debug: Open launch.json”** (или “Добавить конфигурацию”),
* добавьте конфигурацию:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "My Emulator: Launch",
      "type": "my-emulator",
      "request": "launch",
      "program": "${workspaceFolder}/program.emu"
    }
  ]
}
```

`program` — путь к файлу `program.emu` (можно изменить под свою структуру).

### 5.4. Отладка

1. Откройте `program.emu`.
2. Поставьте одну или несколько точек останова (breakpoint) по строкам с инструкциями.
3. В боковой панели **Run and Debug** выберите конфигурацию
   **“My Emulator: Launch”**.
4. Нажмите **Start Debugging** (зелёный треугольник) или F5.

Во время отладки:

* **Continue (F5)** — выполняет программу до следующего breakpoint или конца;
* **Step Over (F10)** — выполняет одну инструкцию;
* в панели **VARIABLES** отображаются:

  * `pc` — текущий индекс инструкции;
  * `acc` — значение аккумулятора;
* в **CALL STACK** отображается один фрейм `main`;
* в **Debug Console** видны служебные сообщения адаптера:

  * запуск сессии,
  * установка breakpoint’ов,
  * остановка на breakpoint,
  * завершение программы.

---

## 6. Типичный сценарий использования

1. Пользователь клонирует репозиторий.
2. Собирает C++ проект через CMake (`cmake ..`, `cmake --build .`).
3. Собирает и настраивает VS Code-расширение (`npm install`, `npm run compile`, настраивает путь к `debug_adapter`).
4. Запускает Extension Development Host (F5 в `vscode-extension`).
5. В нём открывает папку с `program.emu`, создаёт `launch.json`.
6. Запускает отладку и управляет выполнением программы через стандартные кнопки VS Code.

---

## 7. Ограничения и заметки

* Эмулятор очень простой: нет памяти, нет стека, только `pc` и `acc`.
* Каждая строка в `program.emu` соответствует одной инструкции и напрямую мапится на `pc`:

  * строка 1 → `pc = 0`,
  * строка 2 → `pc = 1`, и т.д.
* Debug Adapter реализует только базовые команды DAP (`initialize`, `launch`, `setBreakpoints`, `continue`, `next`, `threads`, `stackTrace`, `scopes`, `variables`, `disconnect`).

