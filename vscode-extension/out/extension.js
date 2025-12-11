"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = require("vscode");
const path = require("path");
function activate(context) {
    console.log('my-emulator debugger extension activated');
    const factory = new class {
        createDebugAdapterDescriptor(session) {
            const exeName = process.platform === 'win32'
                ? 'debug_adapter.exe'
                : 'debug_adapter';
            const command = path.join(context.extensionPath, 'bin', exeName);
            console.log(`My Emulator Debugger: using adapter at ${command}`);
            return new vscode.DebugAdapterExecutable(command, []);
        }
    };
    context.subscriptions.push(vscode.debug.registerDebugAdapterDescriptorFactory('my-emulator', factory));
}
function deactivate() {
    console.log('my-emulator debugger extension deactivated');
}
//# sourceMappingURL=extension.js.map