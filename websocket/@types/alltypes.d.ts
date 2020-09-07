declare module "action-cable-node"
declare module "graphql-ruby-client/subscriptions/ActionCableLink"
declare module "node-fetch"
declare module "dotenv"
declare module "node-native-printer"
declare module "apollo-utilities"
declare module "jwt-decode" {
    interface OptionsType {
        header: boolean;
    }
    type jwtDecodeType = (token: string, options?: OptionsType) => { printTerminalId: string };
    const jwtDecode: jwtDecodeType;
    export default jwtDecode;
}
