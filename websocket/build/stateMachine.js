"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var StateMachine = /** @class */ (function () {
    function StateMachine() {
        this.currentState = 'idle';
    }
    StateMachine.prototype.updateState = function (newState) {
        this.currentState = newState;
    };
    StateMachine.prototype.idle = function () {
        this.currentState = 'idle';
    };
    StateMachine.prototype.holdForScanJob = function () {
        this.currentState = 'holdForScanJob';
    };
    StateMachine.prototype.scanLocal = function () {
        this.currentState = 'scanLocal';
    };
    return StateMachine;
}());
exports.default = StateMachine;
