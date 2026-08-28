<template>
  <div class="grid grid-cols-1">
    <div class="setting_row">
      <p>{{ t('IOBase.VI.defaultValue') }}</p>
      <div class="input">
        <input
          :id="`sw_${index}_defaultValue`"
          :class="['', defValueUnvalid ? 'validation_error' : '']"
          type="number"
          v-model="swi.defaultValue"
          @change="validate()"
        />
      </div>
    </div>
    <div class="setting_row">
      <p>{{ t('IOBase.VI.expiration') }}</p>
      <div class="input_with_unit">
        <span class="unit">sec</span>
        <input
          :id="`sw_${index}_expiration`"
          :class="['', expirationUnvalid ? 'validation_error' : '']"
          type="number"
          v-model="swi.expiration"
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

const { isGreaterThan, isLowerThan, isNegative } = useValidator();

const props = defineProps({
  t: Function,
  index: Number,
  swi: Object,
});

const emit = defineEmits(["update:validated", "update:pinUsed"]);

let defValueUnvalid = ref(false);
let expirationUnvalid = ref(false);

const validate = () => {
  emit("update:validated", { index: props.index, isValid: false });

  props.swi.defaultValue = parseInt(props.swi.defaultValue);
  props.swi.expiration = parseInt(props.swi.expiration);

  defValueUnvalid.value = false;
  expirationUnvalid.value = false;
  if (isLowerThan(props.swi.defaultValue, -2147483648)) {
    defValueUnvalid.value = true;
    const errorMessage = props.t('errors.general.lowerThanValue') + "-2147483648"
    errorResponseNotify(errorMessage);
    return;
  }
  if (isGreaterThan(props.swi.defaultValue, 2147483647)) {
    defValueUnvalid.value = true;
    const errorMessage = props.t('errors.general.greaterThan') + "2147483647"
    errorResponseNotify(errorMessage);
    return;
  }
  if (isNegative(props.swi.expiration)) {
    expirationUnvalid.value = true;
    errorResponseNotify(props.t('errors.general.negativeValue'));
    return;
  }

  emit("update:validated", { index: props.index, isValid: true });
};

const errorResponseNotify = (errorMessage) => {
  toast.error(errorMessage, {
    autoClose: 3000,
  });
};

onMounted(() => {
  props.swi.defaultValue = props.swi.defaultValue ?? 0;
  props.swi.expiration = props.swi.expiration ?? 0;

  validate();
});
</script>
