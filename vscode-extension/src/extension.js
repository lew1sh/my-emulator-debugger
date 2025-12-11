"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
var vscode = require("vscode");
function activate(context) {
    console.log('my-emulator debugger extension activated');
    var factory = new /** @class */ (function () {
        function class_1() {
        }
        class_1.prototype.createDebugAdapterDescriptor = function (session) {
            // Путь к C++ binary debug_adapter
            var command = "/Users/tim/Desktop/my-vm-debugger/build/debug_adapter";
            return new vscode.DebugAdapterExecutable(command, []);
        };
        return class_1;
    }());
    context.subscriptions.push(vscode.debug.registerDebugAdapterDescriptorFactory('my-emulator', factory));
}
function deactivate() {
    console.log('my-emulator debugger extension deactivated');
}
