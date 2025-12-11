import * as vscode from 'vscode';

export function activate(context: vscode.ExtensionContext) {
    console.log('my-emulator debugger extension activated');

    const factory = new class implements vscode.DebugAdapterDescriptorFactory {
        createDebugAdapterDescriptor(
            session: vscode.DebugSession
        ): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {

            // Берём путь к debug_adapter из конфигурации запуска
            const cfg: any = session.configuration;
            const command: string | undefined = cfg.debugAdapterPath;

            if (!command) {
                vscode.window.showErrorMessage(
                    "My Emulator Debugger: 'debugAdapterPath' is not set in launch.json"
                );
                return undefined;
            }

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
