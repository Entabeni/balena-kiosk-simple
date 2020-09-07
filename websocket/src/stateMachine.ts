class StateMachine {
  private currentState: string
  constructor() {
    this.currentState = 'idle';
  }
  updateState(newState: string) {
    this.currentState = newState;
  }

  idle() {
    this.currentState = 'idle';
  }
  holdForScanJob() {
    this.currentState = 'holdForScanJob';
  }
  scanLocal() {
    this.currentState = 'scanLocal';
  }
}
export default StateMachine;
