import * as vscode from 'vscode';

export function activate(context: vscode.ExtensionContext) {
    console.log('my-emulator debugger extension activated');

    const factory = new class implements vscode.DebugAdapterDescriptorFactory {
        createDebugAdapterDescriptor(
            session: vscode.DebugSession
        ): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {

            // Путь к C++ binary debug_adapter
            const command = "/Users/tim/Desktop/my-vm-debugger/build/adapter/debug_adapter";

            return new vscode.DebugAdapterExecutable(command, []);
        }
    };

    context.subscriptions.push(
        vscode.debug.registerDebugAdapterDescriptorFactory('my-emulator', factory)
    );
}

export function deactivate() {
    console.log('my-emulator debugger extension deactivated');
}
