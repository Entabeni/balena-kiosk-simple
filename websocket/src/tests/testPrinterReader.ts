import CardRead from  '../printers/CardRead'
const cardReader = new CardRead('ttyPrinterReader');
cardReader.startScanning();
cardReader.readCardNumber(cardNumberHex => {
  console.log('TCL: cardNumberHex', cardNumberHex);
  const last8Digits = cardNumberHex.substr(
    cardNumberHex.length - 8,
    cardNumberHex.length
  );
});
