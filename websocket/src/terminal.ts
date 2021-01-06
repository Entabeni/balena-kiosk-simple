import TokenService from "./tokenService";
import WebSocket from "./ws";
// import StateMachine from "./stateMachine";
// import MessageQueue from "./messageQueue";
import CardPrinter from "./printers/CardPrinter";
import ReceiptPrinter from "./printers/ReceiptPrinter";
import CardScanner from "./printers/CardScanner";
import LabelPrinter from "./printers/LabelPrinter";
// export {};

function main() {
  // const state = new StateMachine();
  const ws = new WebSocket();
  const ts = new TokenService();
  const receiptPrintObj = new ReceiptPrinter(ws, "receiptPrintJobs");
  const cardScanObj = new CardScanner(ws, "scanJobs");
  const cardPrintPObj = new CardPrinter(ws, "cardPrintJobs");
  ts.connect((token, baseUrl) => {
    ws.subscribe(token, baseUrl, receiptPrintObj, cardScanObj, cardPrintPObj);
  });
}

main();
