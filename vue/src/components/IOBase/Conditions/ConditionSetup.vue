<template>
  <div class="grid grid-cols-4 bg-stone-900 mb-2">
    <div class="setting_row">
      <p>{{ t('safety.Conditions.setup.name') }}</p>
          <input
            type="text"
            :id="`cnd_${index}_name`"
            class="w-full identifier"
            v-model="condition.name"
            maxlength="30"
            @input="onNameInput"
          />
    </div>
    <div class="setting_row">
      <p>{{ t('safety.Conditions.setup.swName') }}</p>
        <select
          :id="`cnd_${index}_switch`"
          :class="{ validation_error: switchUnvalid }"
          v-model="condition.uniqueId"
          @change="onFieldChange"
        >
          <option
            v-for="s in switchRefs"
            :key="s.uniqueId"
            :value="s.uniqueId"
          >
            {{ s.name }}
          </option>
        </select>

    </div>
    <div class="setting_row">
      <p>{{ t('safety.Conditions.setup.checkType') }}</p>
        <select
          :id="`cnd_${index}_checkType`"
          :class="{ validation_error: checkTypeUnvalid }"
          v-model.number="condition.ckType"
          @change="onFieldChange"
        >
            <option value=0>{{ t("safety.Conditions.setup.checkTypeEnum.min") }}</option>
            <option value=1>{{ t("safety.Conditions.setup.checkTypeEnum.minEq") }}</option>
            <option value=2>{{ t("safety.Conditions.setup.checkTypeEnum.Equal") }}</option>
            <option value=3>{{ t("safety.Conditions.setup.checkTypeEnum.GreEq") }}</option>
            <option value=4>{{ t("safety.Conditions.setup.checkTypeEnum.Greater") }}</option>
        </select>
    </div>
    <div class="setting_row">
      <p>{{ t('safety.Conditions.setup.checkValue') }}</p>
          <input
            type="number"
            :class="{ validation_error: refValueUnvalid }"
            :id="`cnd_${index}_refValue`"
            class="w-full identifier"
            v-model.number="condition.refValue"
            @change="onFieldChange"
          />
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue';
import { toast } from "vue3-toastify";

import { useValidator } from "../../../composables/Validator.js";

const { isGreaterThan, isNegative } = useValidator();

const props = defineProps({
  t: Function,
  condition: Object,
  switchRefs: Array,
  index: Number
});

const emit = defineEmits(['changed','validated']);
let switchUnvalid = ref(false);
let checkTypeUnvalid = ref(false);
let refValueUnvalid = ref(false);




const onFieldChange = () => {
  validate()
  emit('changed', { ...props.condition });
};

const onNameInput = (e) => {
  const raw = e.target.value;
  const cleaned = raw.replace(/[<>#!?*]/g, "").slice(0, 20);
  e.target.value = cleaned;
  emit('changed', { ...props.condition, name: cleaned }); 
}
onMounted(() => {
  props.condition.name = props.condition.name ?? "";
  props.condition.uniqueId = props.condition.uniqueId ?? "";
  props.condition.refValue = props.condition.refValue ?? 0;
  props.condition.ckType = props.condition.ckType ?? 0;

  validate();

});

const selectedSwitch = computed(() =>
  props.switchRefs.find(s => s.uniqueId === props.condition.uniqueId) ?? null
);

const isTypeOf = computed(() => {
  return selectedSwitch.value?.type;

});


const validate = () => {

  emit('validated', { index: props.index, isValid: false });

  switchUnvalid.value = false;
  checkTypeUnvalid.value = false;
  refValueUnvalid.value = false;

  if (!selectedSwitch.value) {
    switchUnvalid.value = true;
    errorResponseNotify(props.t('errors.condition.switchNotPresent'))
    return;
  }

  let switchType = isTypeOf.value

  props.condition.ckType = parseInt(props.condition.ckType);
  props.condition.refValue = parseInt(props.condition.refValue);

  if(switchType === 1 ||  switchType === 2){

    if(props.condition.ckType != 2){
      checkTypeUnvalid.value= true;
      errorResponseNotify(props.t('errors.condition.checkTypeBinary'))
      return
    }

    if(props.condition.refValue != 0 && props.condition.refValue != 1){
      refValueUnvalid.value= true;
      errorResponseNotify(props.t('errors.condition.outsideBinaryValue'))
      return
    }

  }

  if(switchType == 4){
    if(props.condition.refValue < 0 || props.condition.refValue > 4095){
      refValueUnvalid.value= true;
      errorResponseNotify(props.t('errors.condition.outsidePWMValue'))
      return
    }
  }

  if(switchType == 5){
    if(props.condition.refValue < -2147483648){
      refValueUnvalid.value= true;
      errorResponseNotify(props.t('errors.condition.outsideVirtualMinValue'))
      return
    }

        if(props.condition.refValue > 2147483647){
      refValueUnvalid.value= true;
      errorResponseNotify(props.t('errors.condition.outsideVirtualGreValue'))
      return
    }
  }

  emit('validated', { index: props.index, isValid: true });

}


const errorResponseNotify = (errorMessage) => {
  toast.error(errorMessage, {
    autoClose: 3000,
  });
};
</script>
