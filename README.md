
# My Emulator Debugger

Учебный проект: очень простой эмулятор “виртуальной машины” на C++ + Debug Adapter (DAP) + отладчик для Visual Studio Code.

С помощью этого проекта вы можете:

- запускать и отлаживать программы в текстовом формате `program.emu`;
- ставить точки останова (breakpoints) прямо в этом файле;
- выполнять программу пошагово;
- смотреть значения регистров `pc` (program counter) и `acc` (аккумулятор) в окне VARIABLES VS Code.

Этот README описывает **упрощённый сценарий для пользователей**:  
у вас есть готовое расширение `.vsix`, вы его ставите в свой VS Code и используете.

---

## 1. Что делает эмулятор

Эмулятор — это маленькая виртуальная машина с:

- `pc` — счётчик команд (индекс текущей строки программы),
- `acc` — аккумулятор (целое число),
- `state` — состояние: `Stopped`, `Running`, `Exited`,
- `program` — список инструкций.

Поддерживаемые команды:

- `ADD N` — `acc += N; pc++`
- `SUB N` — `acc -= N; pc++`
- `JMP N` — `pc = N` (переход к инструкции с индексом `N`)
- `HLT 0` — завершить выполнение (`state = Exited`)

Формат файла `program.emu`:

```text
ADD 1
ADD 2
SUB 1
HLT 0
````

Каждая строка → одна инструкция.
**Строка 1** соответствует `pc = 0`, строка 2 → `pc = 1` и т.д.
Поэтому breakpoints по строкам напрямую превращаются в “остановиться на таком-то `pc`”.

---

## 2. Требования

Для использования вам нужно:

* **Visual Studio Code**
* **компилятор C++** (g++, clang, MSVC — что-то одно)
* **CMake ≥ 3.15** (для сборки C++-части)

**npm / Node.js** нужны только тем, кто сам будет собирать расширение из исходников.
Если вы ставите готовый `.vsix` — npm вам не обязателен.

---

## 3. Быстрый обзор архитектуры (одним абзацем)

* **Эмулятор (C++)** — исполняет инструкции и хранит состояние (`pc`, `acc`, breakpoints, состояние).
* **Debug Adapter (C++)** — процесс `debug_adapter`, который разговаривает с VS Code по Debug Adapter Protocol (DAP): читает JSON-запросы, вызывает методы эмулятора, отправляет JSON-ответы и события (`stopped`, `terminated`, `output`, `stackTrace`, `variables` и т.д.).
* **Расширение VS Code (.vsix)** — говорит VS Code: “у меня есть новый тип отладчика `my-emulator`, вот как его запускать”, и при старте создаёт `DebugAdapterExecutable`, указывая на бинарник `debug_adapter`.

---

## 4. Установка расширения из `.vsix`

### 4.1. Получить `.vsix`

Скачайте файл вида:

```text
my-emulator-debugger-0.0.1.vsix
```

из Releases этого репозитория (или из того места, где вам его дали).

### 4.2. Установить в VS Code

1. Откройте **обычный** Visual Studio Code.
2. Перейдите на вкладку **Extensions** (иконка кубика слева).
3. Нажмите на кнопку с тремя точками `⋯` (More Actions).
4. Выберите **Install from VSIX…**.
5. Укажите файл `my-emulator-debugger-0.0.1.vsix`.
6. Подтвердите установку и перезапустите VS Code, если потребуется.

После этого:

* VS Code **знает тип отладчика** `"my-emulator"`;
* в `launch.json` больше не будет ошибки `Configured debug type 'my-emulator' is not supported`.

---

## 5. Сборка C++-части (эмулятор + debug adapter)

Эмулятор и debug adapter должны быть собраны **локально**, чтобы расширение могло их запускать.

### 5.1. Клонирование репозитория

```bash
git clone https://github.com/lew1sh/my-emulator-debugger.git
cd my-emulator-debugger
```

### 5.2. Сборка через CMake

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

После успешной сборки должен появиться бинарник debug adapter, например:

* на Linux/macOS:
  `build/adapter/debug_adapter`
* на Windows:
  `build/adapter/Debug/debug_adapter.exe` или похожий путь

**Запомните абсолютный путь** к этому файлу — он нужен в `launch.json`.

---

## 6. Подготовка тестовой программы

Откройте отдельную папку для своего кода (это может быть сам репозиторий или любая другая директория) и создайте файл `program.emu`:

```text
ADD 1
ADD 2
SUB 1
HLT 0
```

Убедитесь, что:

* файл открыт в VS Code,
* в статусной строке (справа внизу) язык файла — **Plain Text**
  (если нет, кликните и выберите “Plain Text”).

---

## 7. Настройка `launch.json`

В этой же папке откройте/создайте `.vscode/launch.json`.

Пример конфигурации:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "My Emulator: Launch",
      "type": "my-emulator",
      "request": "launch",

      "program": "${workspaceFolder}/program.emu",
      "debugAdapterPath": "/ABSOLUTE/PATH/TO/my-emulator-debugger/build/adapter/debug_adapter"
    }
  ]
}
```

Где:

* `"program"` — путь к вашему файлу `program.emu`;
* `"debugAdapterPath"` — **абсолютный путь** к бинарнику `debug_adapter`,
  который вы собрали в разделе 5. Например:

  ```json
  "debugAdapterPath": "/Users/username/projects/my-emulator-debugger/build/adapter/debug_adapter"
  ```

Если `debugAdapterPath` не указан или указан неверно, расширение покажет ошибку:

> My Emulator Debugger: 'debugAdapterPath' is not set in launch.json

---

## 8. Запуск отладки

1. Откройте свою рабочую папку в VS Code (ту, где лежит `program.emu` и `.vscode/launch.json`).
2. Откройте файл `program.emu`.
3. Поставьте breakpoint (красный кружок слева от номера строки) на одной или нескольких строках.
4. Перейдите на вкладку **Run and Debug** (зелёный треугольник слева).
5. В списке конфигураций выберите **My Emulator: Launch**.
6. Нажмите **F5** (Start Debugging).

Что будет:

* расширение запустит бинарник `debug_adapter`, указанный в `debugAdapterPath`;
* VS Code начнёт общаться с ним по Debug Adapter Protocol (DAP);
* debug adapter загрузит `program.emu`, запустит эмулятор и сразу остановится на входе (`reason: "entry"`).

Дальше можно:

* нажимать **Continue (F5)** — выполнение идёт до следующего breakpoint или до конца;
* нажимать **Step Over (F10)** — выполняется одна инструкция;
* в окне **VARIABLES** видеть:

  * `pc` — текущий индекс инструкции (0 = первая строка),
  * `acc` — значение аккумулятора;
* в окне **CALL STACK** видеть фрейм `main`;
* в **Debug Console** видеть служебные сообщения адаптера:

  * старт сессии,
  * установка breakpoint’ов,
  * остановка на breakpoint,
  * завершение программы.

---

## 9. Типичный сценарий “от начала до конца”

1. Поставили VS Code.

2. Установили расширение из `my-emulator-debugger-0.0.1.vsix`.

3. Клонировали репозиторий и собрали C++:

   ```bash
   cd my-emulator-debugger
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

4. Запомнили путь к `build/adapter/debug_adapter`.

5. В любой удобной папке создали `program.emu`.

6. Создали `.vscode/launch.json` с `"type": "my-emulator"` и указали `debugAdapterPath`.

7. Запустили отладку, поставили breakpoint’ы, посмотрели `pc`/`acc`.

---

## 10. Частые вопросы / проблемы

### 10.1. В `launch.json` подчёркивает `"type": "my-emulator"`

Сообщение `Configured debug type 'my-emulator' is not supported` означает, что:

* расширение **не установлено**,
  или
* установлено не в тот VS Code (например, одно окно — обычный VS Code, другое — другой профиль).

Решение:

* убедитесь, что `.vsix` установлен в этот VS Code (см. раздел 4);
* перезапустите VS Code после установки расширения.

---

### 10.2. При запуске отладки ошибка про `debugAdapterPath`

Проверьте:

* в `launch.json` есть поле `"debugAdapterPath"`;
* путь **абсолютный**;
* файл там действительно существует и исполняемый (на Linux/macOS: `chmod +x debug_adapter` при необходимости).

---

### 10.3. `npm install` пишет про vulnerabilities

Если вы **просто пользователь и ставите `.vsix`**, вам не нужно запускать `npm install` вообще.

`npm install` и предупреждения про `vulnerabilities` имеют значение только для тех, кто **сам собирает расширение из исходников**. Для запуска готового `.vsix` это не важно.

---

## 11. Для разработчиков (очень коротко)

Если вы хотите не только использовать, но и править расширение:

1. В `vscode-extension/`:

   ```bash
   cd vscode-extension
   npm install
   npm run compile
   ```

2. Меняете код в `src/extension.ts`.

3. Снова `npm run compile` → обновляется `out/extension.js`.

4. Чтобы собрать новый `.vsix`:

   ```bash
   npm install -g @vscode/vsce
   vsce package
   ```

   Появится новый файл `my-emulator-debugger-<version>.vsix`.

Основной сценарий для обычного пользователя описан в разделах 4–9.

