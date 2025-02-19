export class Validator {

    constructor(value,divClass) {
        this.value = value;
        this.divClass = divClass
        this.errors = [];
    }

    static value(val,divClass) {
        return new Validator(val,divClass);
    }

    negativeValue() {
        if (Number.isNaN(this.value) || this.value < 0) {
            this.errors.push("Value must be non-negative");
        }
        return this;
    }

    greaterThan(maxValue) {
        if (this.value > maxValue) {
            this.errors.push(`Value cannot be greater than ${maxValue}`);
        }
        return this;
    }

    isInvalidPin(type = "input") {
        const noUsablePin = [6, 7, 8, 9, 10, 11, 20, 24, 28, 29, 30, 31, 37, 38];
        const noUsableOutputPin = [3, 34, 35, 36, 39];
        const noUsableInputPin = [1];

        let invalidPins = [...noUsablePin];
        if (type === "output") invalidPins.push(...noUsableOutputPin);
        if (type === "input") invalidPins.push(...noUsableInputPin);

        if (isNaN(this.value) || this.value == null || invalidPins.includes(this.value) || this.value < 0 || this.value > 39) {
            this.errors.push(`PIN ${this.value} is not valid as ${type}`);
        }
        return this;
    }

    evaluate() {
        if (this.errors.length > 0) {
            if (this.divClass) {
                    let el = document.getElementById(this.divClass);
                    if (el) {
                        el.classList.add('validation_error');
                        el.scrollIntoView({ behavior: 'smooth', block: 'center' });
                    } else {
                        console.error("Element not found after timeout");
                    }
                }
            console.error("Validation errors:", this.errors);
            return true;
        }

        if (this.divClass) {
            const el = document.getElementById(this.divClass);
            if (el) el.classList.remove('validation_error');
        }
        return false;
    }
}