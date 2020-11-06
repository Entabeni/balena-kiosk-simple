import CardRead from "../printers/CardRead";

const cardReader = new CardRead("ttyDesktopReader");
cardReader.startScanning();
cardReader.readCardNumber((cardNumberHex) => {
  const last8Digits = cardNumberHex.substr(
    cardNumberHex.length - 8,
    cardNumberHex.length
  );
});
