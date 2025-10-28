<template>
  <Card
    v-if="props.txt.switch"
    :moduleName="props.txt.coverC.title"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <p class="title">Configurazione Modulo</p>
    <div class="card mb-4">
      <div class="setting_table">
        <div class="flex">
          <div class="txt pr-4">
            {{ props.txt.gen?.moduleIs }}
          </div>
          <div class="module_toggle">
            <label class="toggle" for="coverc_modlue_state">
              <input
                class="toggle__input"
                name=""
                type="checkbox"
                id="coverc_modlue_state"
                v-model="coverC.enable"
                @change="validate()"
              />
              <div class="toggle__fill"></div>
            </label>
          </div>
        </div>

        <div class="txt pr-4">
          Posizione nella homepage:
          <select id="board_locale" v-model="coverC.uiOrder">
            <option :value="1">1</option>
            <option :value="2">2</option>
            <option :value="3">3</option>
          </select>
        </div>
        <div class="txt pr-4">
          Nome modulo:
          <input
            :id="`coverc_module_name`"
            type="text"
            v-model="coverC.identifier"
          />
        </div>
      </div>
    </div>
    <div v-if="coverC.enable">
      <div class="my-4 flex justify-evenly">
        <p class="font-bold">{{ props.txt.coverC.calibrator }}</p>
        <label class="toggle" for="coverc_calib_present">
          <input
            class="toggle__input"
            name=""
            type="checkbox"
            id="coverc_calib_present"
            v-model="coverC.calibrator.enable"
            @change="validate()"
          />
          <div class="toggle__fill"></div>
        </label>
      </div>

      <div class="card" v-if="coverC.calibrator.enable">
        <PWM
          :txt="props.txt"
          :index="0"
          :swi="coverC.calibrator.outPWM"
          @update:validated="handleValidation"
          @update:pinUsed="updateCalibratorPin"
        />
      </div>

      <div class="my-4 flex justify-evenly">
        <p class="font-bold">{{ props.txt.coverC.cover }}</p>
        <label class="toggle" for="coverc_cover_present">
          <input
            class="toggle__input"
            name=""
            type="checkbox"
            id="coverc_cover_present"
            v-model="coverC.cover.enable"
            @change="validate()"
          />
          <div class="toggle__fill"></div>
        </label>
      </div>

      <div class="card" v-if="coverC.cover.enable">
        <div class="grid grid-cols-2">
          <div>
            <div class="setting_row">
              <p>Pos. apertura</p>
              <div class="input_with_unit">
                <span class="unit">%</span
                ><input
                  :id="`cover_open_pos`"
                  :class="['with_unit', openPosValid ? 'validation_error' : '']"
                  type="number"
                  v-model="coverC.cover.openPos"
                  @change="validate()"
                />
              </div>
            </div>
            <div class="setting_row">
              <p>Pos. chiusura</p>
              <div class="input_with_unit">
                <span class="unit">%</span
                ><input
                  :id="`cover_close_pos`"
                  :class="[
                    'with_unit',
                    closePosValid ? 'validation_error' : '',
                  ]"
                  type="number"
                  v-model="coverC.cover.closePos"
                  @change="validate()"
                />
              </div>
            </div>
          </div>

          <div>
            <Servo
              :txt="props.txt"
              :index="1"
              :swi="coverC.cover.outServo"
              @update:validated="handleValidation"
              @update:pinUsed="updateCoverPin"
            />
          </div>
        </div>
      </div>
    </div>

    <div class="config_buttons">
      <button class="green cursor-pointer" @click="getOriginal()">
        {{ props.txt.gen?.loadFromBoard }}
      </button>
      <button
        :class="[
          validationState ? 'red cursor-pointer' : 'black cursor-not-allowed',
        ]"
        @click="saveData()"
      >
        {{ props.txt.gen?.save }}
      </button>
    </div>
  </Card>
</template>

<script setup>
import { ref, onMounted, watch } from "vue";
import { toast } from "vue3-toastify";
import Card from "../Card.vue";
import PWM from "../IOBase/PWM.vue";
import Servo from "../IOBase/Servo.vue";
import { useValidator } from "../../composables/Validator";

const { isGreaterThan, isNegative } = useValidator();

const props = defineProps({
  txt: Object,
  reboot: Boolean,
});

const emit = defineEmits(["update:reboot", "update:pinUsed"]);

const coverC = ref({});
const pinUsed = ref([]);

const originalData = ref({});
let dataLoaded = ref(false);
let statusClass = ref("green");
let openPosValid = ref(true);
let closePosValid = ref(true);
let validation = ref([]);
let validationState = ref(true);

const handleValidation = (data) => {
  const { index, isValid } = data;
  validation.value[index] = isValid;
};

const validate = () => {
  validationState.value = false;
  statusClass.value = "red";

  if (coverC.value.cover.enable) {
    coverC.value.cover.openPos = parseInt(coverC.value.cover.openPos);
    openPosValid.value = false;
    if (isNegative(coverC.value.cover.openPos)) {
      openPosValid.value = true;
      errorResponseNotify(props.txt.errors.general.negativeValue);
      return;
    }
    if (isGreaterThan(coverC.value.cover.openPos, 100)) {
      openPosValid.value = true;
      const errorMessage = props.txt.errors.general.greaterThan + " 100";
      errorResponseNotify(errorMessage);
      return;
    }

    coverC.value.cover.closePos = parseInt(coverC.value.cover.closePos);
    closePosValid.value = false;
    if (isNegative(coverC.value.cover.closePos)) {
      closePosValid.value = true;
      errorResponseNotify(props.txt.errors.general.negativeValue);
      return;
    }
    if (isGreaterThan(coverC.value.cover.closePos, 100)) {
      closePosValid.value = true;
      const errorMessage = props.txt.errors.general.greaterThan + " 100";
      errorResponseNotify(errorMessage);
      return;
    }
  }

  validationState.value = true;
  statusClass.value = "green";
};

const getOriginal = () => {
  coverC.value = JSON.parse(JSON.stringify(originalData.value));
};

const fetchData = async () => {
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + "/api/coverc/cfg");
    if (!response.ok) {
      throw new Error("Network response was not ok");
    }
    const data = await response.json();
    coverC.value = data;
    dataLoaded.value = true;

    if (!coverC.value.cover.outServo) {
      coverC.value.cover.outServo = {};
    }

    if (!coverC.value.calibrator.outPWM) {
      coverC.value.calibrator.outPWM = {};
    }
    updatePinsforObserver(data);
    setupWatch();
    if (data.reboot) {
      statusClass.value = "orange";
    }

    originalData.value = JSON.parse(JSON.stringify(coverC.value));
  } catch (error) {
    console.error("Errore durante la chiamata API:", error);
  }
};

const saveData = async () => {
  if (!validationState.value) {
    errorResponseNotify(props.txt.errors.general.validationFailed);
    return;
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + "/api/coverc/cfg", {
      method: "POST",
      headers: {
        Accept: "application/json, text/plain, */*",
        "Content-Type": "application/json",
      },
      body: JSON.stringify(coverC.value),
    });

    const data = await response.json();
    if (!response.ok) throw { status: response.status, data };
    cmdExecutedNotify();

    if (data.reboot) {
      console.log("reboot needed");
      emit("update:reboot", true);
      statusClass.value = "orange";
    }
  } catch (err) {
    if (err?.status === 500 && Array.isArray(err.data?.errors)) {
      err.data.errors.forEach((e) =>
        typeof e === "object"
          ? handleStructuredError(e)
          : errorResponseNotify(e)
      );
    } else {
      errorResponseNotify(
        err?.message || props.txt.errors.general.configRejected
      );
    }
  }
};

const handleStructuredError = (e) => {
  let msg;
  let pinName;
  switch (e.id) {
    case 1:
      pinName = props.txt.coverC.calibrator;
      break;
    case 2:
      pinName = props.txt.coverC.cover;
      break;
    default:
      pinName = `Pin ${e.id}`;
  }

  msg = pinName + ": " + props.txt.errors.gpioValidation[e.error];
  errorResponseNotify(msg);
};

const cmdExecutedNotify = () => {
  toast.success(props.txt.gen.configSaved, {
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

function setupWatch() {
  watch(
    () => [
      validation.value,
      coverC.value.calibrator.present,
      coverC.value.cover.present,
    ],
    () => {
      if (coverC.value.calibrator.enable && validation.value[0] === false) {
        validationState.value = false;
      } else if (coverC.value.cover.enable && validation.value[1] === false) {
        validationState.value = false;
      } else {
        validationState.value = true;
      }
    },
    { deep: true }
  );
}

const updatePinsforObserver = (data) => {
  pinUsed.value = [];
  pinUsed.value[0] = { pin: data.calibrator.outPWM.pin, type: 3, module: 2 };
  pinUsed.value[1] = { pin: data.cover.outServo.pin, type: 4, module: 2 };
  emit("update:pinUsed", pinUsed.value);
};

const updateCalibratorPin = (data) => {
  pinUsed.value[0] = { pin: data.pin, type: 3, module: 2 };
  emit("update:pinUsed", pinUsed.value);
};

const updateCoverPin = (data) => {
  pinUsed.value[1] = { pin: data.pin, type: 4, module: 2 };
  emit("update:pinUsed", pinUsed.value);
};
</script>
