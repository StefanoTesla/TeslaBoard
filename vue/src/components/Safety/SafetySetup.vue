<template>
  <Card
    v-if="t('safety')"
    :moduleName="t('safety.title')"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <div class="card mb-4">
      <p class="title">{{ t('gen.moduleSetting') }}</p>
      <div class="setting_table">
        <div class="flex">
          <div class="txt pr-4">
            {{ t('gen.moduleIs') }}
          </div>
          <div class="module_toggle">
            <label class="toggle" for="safety_module_status">
              <input
                class="toggle__input"
                name=""
                type="checkbox"
                id="safety_module_status"
                v-model="safety.enable"
                @change="validate()"
              />
              <div class="toggle__fill"></div>
            </label>
          </div>
        </div>

        <div class="txt pr-4">
          {{ t('gen.uiOrder') }}
          <select id="board_locale" v-model.number="safety.uiOrder">
            <option :value="1">1</option>
            <option :value="2">2</option>
            <option :value="3">3</option>
            <option :value="4">4</option>
          </select>
        </div>
        <div class="txt pr-4">
          {{ t('gen.modIdent') }}
          <input
            id="safety_module_name"
            class="identifier"
            type="text"
            v-model="safety.identifier"
          />
        </div>
      </div>
    </div>
    <div v-if="safety.enable">        
      <div class="card">
        <p class="title">{{ t('safety.Conditions.title') }}</p>
        <button class="ml-4 sw_add green" @click="addNewCondition()">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            viewBox="0 0 72 72"
            width="40px"
            height="40px"
          >
            <path
              d="M36,12c13.255,0,24,10.745,24,24c0,13.255-10.745,24-24,24S12,49.255,12,36C12,22.745,22.745,12,36,12z M44,39 c1.657,0,3-1.343,3-3c0-1.657-1.343-3-3-3c-0.329,0-2.426,0-5,0c0-2.574,0-4.672,0-5c0-1.657-1.343-3-3-3c-1.657,0-3,1.343-3,3  c0,0.328,0,2.426,0,5c-2.574,0-4.671,0-5,0c-1.657,0-3,1.343-3,3c0,1.657,1.343,3,3,3c0.329,0,2.426,0,5,0c0,2.574,0,4.672,0,5  c0,1.657,1.343,3,3,3c1.657,0,3-1.343,3-3c0-0.328,0-2.426,0-5C41.574,39,43.671,39,44,39z"
            />
          </svg>
        </button>

        <div class="grid grid-cols-2" v-for="(cnd, index) in safety.Conditions" :key="index">
          <!-- CONDITION -->

          <div class="setting_row">
            <p>{{ t('safety.Conditions.setup.name') }}</p>
                <input
                  type="text"
                  :id="`cnd_${index}_name`"
                  class="w-full identifier"
                  v-model="cnd.name"
                  maxlength="30"
                  @input="onNameInput"
                />
            </div>
            <div class="setting_row">
              <p>{{ t('safety.Conditions.setup.swName') }}</p>
                <select
                  :id="`cnd_${index}_switch`"
                  :class="{ validation_error: switchUnvalid }"
                  v-model="cnd.uniqueId"
                  @change="onFieldChange"
                >
                  <option
                    v-for="s in switches"
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
                  v-model.number="cnd.ckType"
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
                    v-model.number="cnd.refValue"
                    @change="onFieldChange"
                  />
            </div>

            <button class="red" @click="deleteCondition(index)">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              viewBox="0 0 128 128"
              width="20px"
              height="20px"
            >
              <path
                d="M 49 1 C 47.34 1 46 2.34 46 4 C 46 5.66 47.34 7 49 7 L 79 7 C 80.66 7 82 5.66 82 4 C 82 2.34 80.66 1 79 1 L 49 1 z M 24 15 C 16.83 15 11 20.83 11 28 C 11 35.17 16.83 41 24 41 L 101 41 L 101 104 C 101 113.37 93.37 121 84 121 L 44 121 C 34.63 121 27 113.37 27 104 L 27 52 C 27 50.34 25.66 49 24 49 C 22.34 49 21 50.34 21 52 L 21 104 C 21 116.68 31.32 127 44 127 L 84 127 C 96.68 127 107 116.68 107 104 L 107 40.640625 C 112.72 39.280625 117 34.14 117 28 C 117 20.83 111.17 15 104 15 L 24 15 z M 24 21 L 104 21 C 107.86 21 111 24.14 111 28 C 111 31.86 107.86 35 104 35 L 24 35 C 20.14 35 17 31.86 17 28 C 17 24.14 20.14 21 24 21 z M 50 55 C 48.34 55 47 56.34 47 58 L 47 104 C 47 105.66 48.34 107 50 107 C 51.66 107 53 105.66 53 104 L 53 58 C 53 56.34 51.66 55 50 55 z M 78 55 C 76.34 55 75 56.34 75 58 L 75 104 C 75 105.66 76.34 107 78 107 C 79.66 107 81 105.66 81 104 L 81 58 C 81 56.34 79.66 55 78 55 z"
              />
            </svg>
          </button>
          </div>


        </div>

      </div> 

    <div class="config_buttons">
      <button class="green cursor-pointer" @click="getOriginal()">
        {{ t('gen.loadFromBoard') }}
      </button>
      <button
        :class="[
          validationState ? 'red cursor-pointer' : 'black cursor-not-allowed',
        ]"
        @click="saveData()"
      >
        {{ t('gen.save') }}
      </button>
    </div>
  </Card>
</template>

<script setup>
import { ref, onMounted, watch } from "vue";
import { toast } from "vue3-toastify";
import Card from "../Card.vue";

const props = defineProps({
  t: Function,
  reboot: Boolean,
  pinUsed: Array,
});

const safety = ref({});

const originalData = ref({});
let dataLoaded = ref(false);
let statusClass = ref("green");
let validation = ref([]);
let switches = ref([]);
let validationState = ref(true);

const handleValidation = ({ index, isValid }) => {
  validation.value[index] = isValid;
};

const onConditionChanged = (i, newVal) => {
  safety.value.Conditions[i] = newVal;

};

const getOriginal = () => {
  safety.value = JSON.parse(JSON.stringify(originalData.value));
};

const fetchData = async () => {
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + "/api/safety/cfg");
    if (!response.ok) { throw new Error("Network response was not ok"); }
    const data = await response.json();
    safety.value = data;
    dataLoaded.value = true;
    if (data.reboot) { statusClass.value = "orange"; }

    originalData.value = JSON.parse(JSON.stringify(safety.value));

    // check switch del device
    const switchRes = await fetch(ip + "/api/switch/cfg");
    if (!switchRes.ok) throw new Error("switch cfg failed");
    const switchCfg = await switchRes.json();

    if (switchCfg.enable) {
      switches.value = (switchCfg.Switches ?? []).map(s => ({
        uniqueId: s.uniqueId,
        name: s.name,
        type: s.type,
      }));
    } else {
      switches.value = [];
    }


  } catch (error) {
    console.error("Errore durante la chiamata API:", error);
  }
};

const validate = () => {
  validationState.value = false;
  statusClass.value = "red";

  safety.value.uiOrder = parseInt(safety.value.uiOrder);
  if (!safety.value.enable) {
    statusClass.value = "green";
    validationState.value = true;
    return;
  }

  statusClass.value = "green";
  validationState.value = true;
};

const saveData = async () => {
  if (!validationState.value && safety.value.enable) {
    errorResponseNotify(props.t('errors.general.validationFailed'));
    return;
  }


  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + "/api/safety/cfg", {
      method: "POST",
      headers: {
        Accept: "application/json, text/plain, */*",
        "Content-Type": "application/json",
      },
      body: JSON.stringify(safety.value),
    });

    const data = await response.json();
    if (!response.ok) throw { status: response.status, data };

    cmdExecutedNotify();

  } catch (err) {
    if (err?.status === 500 && Array.isArray(err.data?.errors)) {
      err.data.errors.forEach((e) =>
        typeof e === "object"
          ? handleStructuredError(e)
          : errorResponseNotify(e)
      );
    } else {
      errorResponseNotify(
        err?.message || props.t('errors.general.configRejected')
      );
    }
  }
};


const addNewCondition = () => {
  if (safety.value.Conditions.length <= 20) {
    safety.value.Conditions.push({
      name: "",
      uniqueId: "",
      refValue: 0,
      ckType: 0,
    });
    validation.value.push(false);
  } else {
    errorResponseNotify(props.t("errors.safety.limitReached"));
  }
};
const deleteCondition = (index) => {
  safety.value.Conditions.splice(index, 1);
  validation.value.splice(index, 1);
};


const cmdExecutedNotify = () => {
  toast.success(props.t('gen.configSaved'), {
    autoClose: 500,
  });
};

const errorResponseNotify = (errorMessage) => {
  toast.error(errorMessage, {
    autoClose: 3000,
  });
};

onMounted(() => {
  fetchData();
});

watch(
  () => validation.value.some(v => v === false),
  (hasFalse) => {
    validationState.value = !hasFalse;
  }
);


</script>
