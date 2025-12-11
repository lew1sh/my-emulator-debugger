// debug_adapter.cpp
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>      
#include "emulator.h"
#include "json.hpp"

using json = nlohmann::json;

static int g_seq = 1;
static Emulator emulator;
static std::string g_programPath;

// отправка JSON-ответа/события в формате DAP
void sendMessage(const json& message) {
    std::string body = message.dump();
    std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    std::cout.flush();
}

void sendResponse(int requestSeq, const std::string& command, bool success, const json& body = json::object()) {
    json resp = {
        {"seq", g_seq++},
        {"type", "response"},
        {"request_seq", requestSeq},
        {"success", success},
        {"command", command},
        {"body", body}
    };
    sendMessage(resp);
}

void sendEvent(const std::string& event, const json& body = json::object()) {
    json ev = {
        {"seq", g_seq++},
        {"type", "event"},
        {"event", event},
        {"body", body}
    };
    sendMessage(ev);
}

// Обработка запросов DAP
void handleRequest(const json& msg) {
    std::string command = msg.value("command", "");
    int requestSeq = msg.value("seq", 0);

    if (command == "initialize") {
        json body = {
            {"supportsConfigurationDoneRequest", true},
            {"supportsStepBack", false}
        };
        sendResponse(requestSeq, command, true, body);
        sendEvent("initialized");
    }  else if (command == "launch") {
    std::string programPath = msg["arguments"].value("program", "");

    std::vector<Instruction> prog;

    if (!programPath.empty()) {
        g_programPath = programPath;        // <--- запоминаем

        std::ifstream in(programPath);
        if (in) {
            std::string op;
            int arg;
            while (in >> op >> arg) {
                prog.push_back({op, arg});
            }
        }
    }

    if (prog.empty()) {
        prog = {
            {"ADD", 1},
            {"ADD", 2},
            {"SUB", 1},
            {"HLT", 0}
        };

        // если файл не прочитался, всё равно что-то положим:
        if (g_programPath.empty()) {
            g_programPath = "program.emu";  // просто имя, чтобы VS Code не падал
        }
    }

    emulator.loadProgram(prog);
    sendEvent("output", {
        {"category", "console"},
        {"output", "My Emulator Debugger: session started, program loaded\n"}
    });
    sendResponse(requestSeq, command, true);
    
    sendEvent("stopped", {
        {"reason", "entry"},
        {"threadId", 1},
        {"allThreadsStopped", true}
    });
}

 else if (command == "setBreakpoints") {
    // Очищаем старые брейки
    emulator.clearBreakpoints();

    json body;
    body["breakpoints"] = json::array();

    auto args = msg["arguments"];
    auto bps = args["breakpoints"];

    for (auto& bp : bps) {
        int line = bp["line"];   

        int pc = line - 1;       

        emulator.addBreakpoint(pc);

        std::string logMsg = "Set breakpoint at line " + std::to_string(line) +
                             " -> pc " + std::to_string(pc) + "\n";
        sendEvent("output", {
            {"category", "console"},
            {"output", logMsg}
        });

        body["breakpoints"].push_back({
            {"verified", true},
            {"line", line}
        });
    }

    sendResponse(requestSeq, command, true, body);
}

 else if (command == "configurationDone") {
        sendResponse(requestSeq, command, true);
    } else if (command == "continue") {
    
    if (emulator.getState() == EmulatorState::Stopped) {
        emulator.step();

        
        std::string stepMsg = "continue(): initial step, pc=" +
                              std::to_string(emulator.getPC()) +
                              " acc=" + std::to_string(emulator.getAcc()) + "\n";
        sendEvent("output", {
            {"category", "console"},
            {"output", stepMsg}
        });

        // Если после шага программа уже завершилась — сразу выходим
        if (emulator.getState() == EmulatorState::Exited) {
            sendResponse(requestSeq, command, true, {{"allThreadsContinued", true}});
            sendEvent("terminated");
            return;
        }
    }

    //  обычный run() 
    emulator.run();

    // Ответ VS Code на сам запрос continue
    sendResponse(requestSeq, command, true, {{"allThreadsContinued", true}});

    if (emulator.getState() == EmulatorState::Stopped) {
        // Попали НА СЛЕДУЮЩИЙ breakpoint
        sendEvent("stopped", {
            {"reason", "breakpoint"},
            {"threadId", 1},
            {"allThreadsStopped", true}
        });

        std::string msg = "Stopped at breakpoint: pc=" +
                          std::to_string(emulator.getPC()) +
                          " acc=" + std::to_string(emulator.getAcc()) + "\n";
        sendEvent("output", {
            {"category", "console"},
            {"output", msg}
        });
    } else {
        
         sendEvent("output", {
            {"category", "console"},
            {"output", "My Emulator Debugger: program finished, session will terminate\n"}
        });
        sendEvent("terminated");
    }
}


 else if (command == "next") { 
    emulator.step();

    // Лог в Debug Console
    std::string msg = "Step: pc=" + std::to_string(emulator.getPC()) +
                      " acc=" + std::to_string(emulator.getAcc()) + "\n";
    sendEvent("output", {
        {"category", "console"},
        {"output", msg}
    });

    sendResponse(requestSeq, command, true);

    if (emulator.getState() == EmulatorState::Stopped) {
        sendEvent("stopped", {
            {"reason", "step"},
            {"threadId", 1},
            {"allThreadsStopped", true}
        });
    } else {
        sendEvent("terminated");
    }
}

 else if (command == "disconnect") {
        sendResponse(requestSeq, command, true);
        sendEvent("terminated");
        std::exit(0);
    } else if (command == "threads") {
        sendEvent("output", {
        {"category", "console"},
        {"output", "threads() called\n"}
    });
        json body = {
            {"threads", json::array({
                {{"id", 1}, {"name", "main"}}
            })}
        };
        sendResponse(requestSeq, command, true, body);
    }else if (command == "stackTrace") {
    int pc = emulator.getPC();

    json body = {
        {"stackFrames", json::array({
            {
                {"id", 1},
                {"name", "main"},
                {"line", pc + 1},    
                {"column", 1},
                {"source", {
                    {"name", "program.emu"},
                    {"path", g_programPath}   
                }}
            }
        })},
        {"totalFrames", 1}
    };

    sendResponse(requestSeq, command, true, body);
}

 else if (command == "scopes") {
        sendEvent("output", {
        {"category", "console"},
        {"output", "scopes() called\n"}
    });
        json body = {
            {"scopes", json::array({
                {{"name", "Locals"}, {"variablesReference", 1}, {"expensive", false}}
            })}
        };
        sendResponse(requestSeq, command, true, body);
    } else if (command == "variables") {
        sendEvent("output", {
        {"category", "console"},
        {"output", "variables() called, pc=" + std::to_string(emulator.getPC()) +
                   " acc=" + std::to_string(emulator.getAcc()) + "\n"}
    });
        int varsRef = msg["arguments"]["variablesReference"];
        json body;
        if (varsRef == 1) {
            body["variables"] = json::array({
                {{"name", "pc"}, {"value", std::to_string(emulator.getPC())}, {"variablesReference", 0}},
                {{"name", "acc"}, {"value", std::to_string(emulator.getAcc())}, {"variablesReference", 0}}
            });
        } else {
            body["variables"] = json::array();
        }
        sendResponse(requestSeq, command, true, body);
    } else {
        sendResponse(requestSeq, command, false, {{"message", "Unknown command"}});
    }
}

int main() {
    std::ios::sync_with_stdio(false);

    std::string line;
    while (true) {

        int contentLength = 0;
        while (std::getline(std::cin, line) && !line.empty() && line != "\r") {
            if (line.rfind("Content-Length:", 0) == 0) {
                contentLength = std::stoi(line.substr(15));
            }
        }

        if (contentLength <= 0) break;

        std::string body(contentLength, '\0');
        std::cin.read(&body[0], contentLength);
        if (!std::cin) break;

        json msg = json::parse(body, nullptr, false);
        if (msg.is_discarded()) {
            continue;
        }

        std::string type = msg.value("type", "");
        if (type == "request") {
            handleRequest(msg);
        } else {
            // игнорируем события/ответы
        }
    }

    return 0;
}
