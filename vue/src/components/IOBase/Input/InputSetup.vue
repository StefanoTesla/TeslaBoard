<template>
  <div class="grid grid-cols-2">
    <div class="setting_row">
      <p>{{ t('IOBase.pin') }}</p>
      <div class="input_with_unit">
        <span class="unit">n</span
        ><input
          :id="`sw_${index}_pin`"
          :class="['with_unit', pinUnvalid ? 'validation_error' : '']"
          type="number"
          v-model="swi.pin"
          @change="validate()"
        />
      </div>
    </div>
    <div class="setting_row">
      <p>{{ t('IOBase.DI.func') }}</p>
      <select
        :id="`sw_${index}_invert`"
        :class="{ validation_error: invertUnvalid }"
        v-model="swi.invert"
        @change="validate()"
      >
        <option :value="false">{{ t('IOBase.DI.invertEnum.0') }}</option>
        <option :value="true">{{ t('IOBase.DI.invertEnum.1') }}</option>
      </select>
    </div>
    <div class="setting_row">
      <p>{{ t('IOBase.DI.onDelay') }}</p>
      <div class="input_with_unit">
        <span class="unit">ms</span
        ><input
          :id="`sw_${index}_dOn`"
          :class="['with_unit', dOnUnvalid ? 'validation_error' : '']"
          type="number"
          v-model="swi.dOn"
          @change="validate()"
        />
      </div>
    </div>
    <div class="setting_row">
      <p>{{ t('IOBase.DI.offDelay') }}</p>
      <div class="input_with_unit">
        <span class="unit">ms</span
        ><input
          :id="`sw_${index}_dOff`"
          :class="['with_unit', dOffUnvalid ? 'validation_error' : '']"
          type="number"
          v-model="swi.dOff"
          @change="validate()"
        />
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from "vue";
import { toast } from "vue3-toastify";
import { useValidator } from "../../../composables/Validator";

const { isInvalidPin, isNegative } = useValidator();

const props = defineProps({
  t: Function,
  index: Number,
  swi: Object,
});

const emit = defineEmits(["update:validated", "update:pinUsed"]);

let pinUnvalid = ref(false);
let dOnUnvalid = ref(false);
let dOffUnvalid = ref(false);
let invertUnvalid = ref(false);

const validate = () => {
  emit("update:validated", { index: props.index, isValid: false });
  props.swi.pin = parseInt(props.swi.pin);
  pinUnvalid.value = false;

  if (isInvalidPin(props.swi.pin, "input")) {
    pinUnvalid.value = true;
    const errorMessage =
      props.t('gen.pin') +
      " " +
      props.swi.pin +
      " - " +
      props.t('errors.gpio.noUsableAsInput');
    errorResponseNotify(errorMessage);
    return;
  }

  props.swi.dOn = parseInt(props.swi.dOn);
  dOnUnvalid.value = false;
  if (isNegative(props.swi.dOn)) {
    dOnUnvalid.value = true;
    errorResponseNotify(props.t('errors.general.negativeValue'));
    return;
  }

  props.swi.dOff = parseInt(props.swi.dOff);
  dOffUnvalid.value = false;
  if (isNegative(props.swi.dOff)) {
    dOffUnvalid.value = true;
    errorResponseNotify(props.t('errors.general.negativeValue'));
    return;
  }

  emit("update:validated", { index: props.index, isValid: true });
  emit("update:pinUsed", { pin: props.swi.pin, type: 1, index: props.index });
};

const errorResponseNotify = (errorMessage) => {
  toast.error(errorMessage, {
    autoClose: 3000,
  });
};

onMounted(() => {
  props.swi.pin = props.swi.pin ?? null;
  props.swi.dOn = props.swi.dOn ?? 0;
  props.swi.dOff = props.swi.dOff ?? 0;
  props.swi.invert = props.swi.invert ?? 0;

  validate();
});
</script>
