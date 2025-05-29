export function useValidator() {
    const isNegative = (value) => {
        return Number.isNaN(value) || value == undefined || value < 0;
    };

    const isGreaterThan = (value, maxValue) => {
        return value > maxValue;
    };

    const isInvalidPin = (value, type = "input") => {
        const noUsablePin = [0,1,3,6, 7, 8, 9, 10, 11, 20, 24, 28, 29, 30, 31, 37, 38];
        const noUsableOutputPin = [34, 35, 36, 39];
        const noUsableInputPin = [12];

        let invalidPins = [...noUsablePin];
        if (type === "output") invalidPins.push(...noUsableOutputPin);
        if (type === "input") invalidPins.push(...noUsableInputPin);

        return isNaN(value) || value == null || value == undefined || invalidPins.includes(value) || value < 0 || value > 39;
    };

    return {
        isNegative,
        isGreaterThan,
        isInvalidPin
    };
}