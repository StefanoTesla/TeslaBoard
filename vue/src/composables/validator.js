import { ref } from "vue";

// Configurazione per ogni espType.
// Chiave = valore di espType che ritorna /api/cfg.
const INVALID_PINS_BY_ESP = {
  0: {
    base:   [0, 1, 3, 6, 7, 8, 9, 10, 11, 20, 24, 28, 29, 30, 31, 37, 38],
    output: [34, 35, 36, 39],
    input:  [12],
  },
  1: {
    base:   [0,3,6,7,8,9,10,11,12,20,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,43,44,45,46,47,48],
    output: [],
    input:  [],
  },
  // aggiungi qui 2, 3, ... quando servirà
};

const DEFAULT_ESP = 0;

// Stato condiviso (singleton a livello di modulo)
const espType = ref(DEFAULT_ESP);

// Chiamala UNA VOLTA dopo la fetch di /api/cfg
export function setEspType(type) {
  if (INVALID_PINS_BY_ESP[type]) {
    espType.value = type;
  } else {
    console.warn(`[validator] espType sconosciuto: ${type}, uso default ${DEFAULT_ESP}`);
    espType.value = DEFAULT_ESP;
  }
}

export function useValidator() {
  const isNegative = (value) =>
    Number.isNaN(value) || value == undefined || value < 0;

  const isGreaterThan = (value, maxValue) => value > maxValue;
  const isLowerThan  = (value, minValue) => value < minValue;

  const isInvalidPin = (value, type = "input") => {
    const cfg = INVALID_PINS_BY_ESP[espType.value] ?? INVALID_PINS_BY_ESP[DEFAULT_ESP];

    const invalidPins = [...cfg.base];
    if (type === "output") invalidPins.push(...cfg.output);
    if (type === "input")  invalidPins.push(...cfg.input);

    return (
      isNaN(value) ||
      value == null ||
      invalidPins.includes(value) ||
      value < 0 ||
      value > 39
    );
  };

  return { isNegative, isGreaterThan, isLowerThan, isInvalidPin, setEspType };
}