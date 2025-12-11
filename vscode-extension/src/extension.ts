import * as vscode from 'vscode';
import * as path from 'path';

export function activate(context: vscode.ExtensionContext) {
    console.log('my-emulator debugger extension activated');

    const factory = new class implements vscode.DebugAdapterDescriptorFactory {
        createDebugAdapterDescriptor(
            session: vscode.DebugSession
        ): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {

            
            const exeName = process.platform === 'win32'
                ? 'debug_adapter.exe'
                : 'debug_adapter';

            const command = path.join(
                context.extensionPath,
                'bin',
                exeName
            );

            
            console.log(`My Emulator Debugger: using adapter at ${command}`);

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
