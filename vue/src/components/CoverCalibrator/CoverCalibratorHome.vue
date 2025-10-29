<template>
  <Card
    v-if="t('coverC')"
    :moduleName="t('coverC.title')"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <div class="card" v-if="coverC.calibrator.status > 0">
      <div class="title">
        <p>{{ t("coverC.calibrator") }}</p>
      </div>
      <div class="flex items-center justify-center">
        <p>{{ t("coverC.home.coverState") }}</p>
        <span v-if="coverC.calibrator.status == 1">{{
          t("coverC.home.calibEnum.off")
        }}</span>
        <span v-if="coverC.calibrator.status == 3">{{
          t("coverC.home.calibEnum.ready")
        }}</span>
      </div>
      <div class="range xl:max-w-3xl xl:mx-auto">
        <input
          type="range"
          v-model="coverC.calibrator.brightness"
          min="0"
          max="4095"
          step="1"
          @change="calibratorBrightnessChange"
        />
      </div>
      <div class="flex justify-center">
        <p>{{ t("gen.status.actualValue") }}</p>
        <p class="pl-2">{{ coverC.calibrator.brightness }}/4095</p>
      </div>
      <div class="flex justify-around">
        <button
          :class="calibratorPowerOnCmdClass"
          @click="calibratorPowerOnCmd"
        >
          {{ t("gen.action.powerOn") }}
        </button>
        <button
          :class="calibratorPowerOffCmdClass"
          @click="calibratorPowerOffCmd"
        >
          {{ t("gen.action.powerOff") }}
        </button>
      </div>
    </div>
    <div class="card mt-4" v-if="coverC.cover.status > 0">
      <div class="title">
        <p>{{ t("coverC.cover") }}</p>
      </div>
      <div class="flex justify-center">
        <p>{{ t("coverC.home.coverState") }}</p>
        <p class="pl-2">{{ coverStatus }}</p>
      </div>
      <div class="flex justify-around">
        <button :class="coverOpenCmdClass" @click="coverOpenCmd">
          {{ t("gen.action.open") }}
        </button>
        <button class="red cursor-pointer" @click="coverHalt">
          {{ t("gen.action.halt") }}
        </button>
        <button :class="coverCloseCmdClass" @click="coverCloseCmd">
          {{ t("gen.action.close") }}
        </button>
      </div>
      <div class="flex justify-center">
        <p>{{ t("gen.status.actualPos") }}</p>
        <p class="pl-2">{{ coverC.cover.angle }}°</p>
      </div>
    </div>
  </Card>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from "vue";
import { toast } from "vue3-toastify";
import Card from "../Card.vue";

const props = defineProps({
  t: Function,
});


let pollingTimeout = null;
let isPolling = false;
let abortController = null;

const coverC = ref({});
let dataLoaded = ref(false);
let statusClass = ref("red");
let canOpenCover = ref(false);
let canCloseCover = ref(false);
let coverStatus = ref("");


const fetchData = async () => {

  if (abortController) {
    abortController.abort();
  }

  abortController = new AbortController();

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + "/api/coverc/status", {
      signal: abortController.signal,
    });

    if (!response.ok) {
      throw new Error("Network response was not ok");
    }

    const data = await response.json();
    coverC.value = data;
    dataLoaded.value = true;
    updateStatusData();
  } catch (error) {
    if (error.name === "AbortError") {
      console.log("Fetch aborted");
      return;
    }
    console.error("Errore durante la chiamata API:", error);
  } finally {
    if (isPolling) {
      pollingTimeout = setTimeout(fetchData, 3000);
    }
  }
};


const startPolling = () => {
  isPolling = true;
  fetchData(); // Prima chiamata immediata
};

const stopPolling = () => {
  isPolling = false;
  if (pollingTimeout) {
    clearTimeout(pollingTimeout);
    pollingTimeout = null;
  }
  if (abortController) {
    abortController.abort();
    abortController = null;
  }
};

const updateStatusData = () => {
  if (coverC.value.cover.status >= 4) {
    statusClass.value = "red";
  } else if (coverC.value.cover.status == 2) {
    statusClass.value = "orange";
  } else if (
    coverC.value.cover.status == 0 &&
    coverC.value.calibrator.status == 0
  ) {
    statusClass.value = "black";
  } else {
    statusClass.value = "green";
  }

  canOpenCover.value =
    coverC.value.cover.status == 2 || coverC.value.cover.status == 3
      ? false
      : true;

  canCloseCover.value = coverC.value.cover.status <= 2 ? false : true;

  const enumCommand = [
    props.t("coverC.home.coverEnum.notPresent"),
    props.t("gen.status.close"),
    props.t("coverC.home.coverEnum.moving"),
    props.t("gen.status.open"),
    props.t("coverC.home.coverEnum.unknow"),
    props.t("coverC.home.coverEnum.error"),
  ];
  coverStatus.value = enumCommand[coverC.value.cover.status];
};

const coverOpenCmdClass = computed(() => {
  return canOpenCover.value
    ? ["green", "cursor-pointer"]
    : ["disactivated", "cursor-not-allowed"];
});

const coverCloseCmdClass = computed(() => {
  return canCloseCover.value
    ? ["green", "cursor-pointer"]
    : ["disactivated", "cursor-not-allowed"];
});

const calibratorPowerOffCmdClass = computed(() => {
  return coverC.value.calibrator.brightness != 0
    ? ["green", "cursor-pointer"]
    : ["disactivated", "cursor-not-allowed"];
});

const calibratorPowerOnCmdClass = computed(() => {
  return coverC.value.calibrator.brightness == 0
    ? ["green", "cursor-pointer"]
    : ["disactivated", "cursor-not-allowed"];
});

const sendCommand = async (endpoint, canExecute = true, errorMsg = null) => {
  if (!canExecute) {
    cmdRefusedNotify();
    return;
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + endpoint, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Accept: "application/json, text/plain, */*",
      },
    });

    if (!response.ok) {
      throw new Error("Network response was not ok");
    }

    const res = await response.json();

    if (res.error) {
      errorResponseNotify(res.error);
      return false;
    }

    if (res.execute) {
      cmdExecutedNotify();
      return true;
    }

    return false;
  } catch (error) {
    noResponseNotify(error);
    return false;
  }
};

const coverOpenCmd = async () => {
  await sendCommand("/api/coverc/open", canOpenCover.value);
};
const coverHalt = async () => {
  await sendCommand("/api/coverc/halt", true);
};

const coverCloseCmd = async () => {
  await sendCommand("/api/coverc/close", canCloseCover.value);
};

const calibratorPowerOnCmd = async () => {
  const canExecute = coverC.value.calibrator.brightness != 4095;
  const success = await sendCommand("/api/coverc/on", canExecute);
  if (success) {
    coverC.value.calibrator.brightness = 4095;
  }
};

const calibratorPowerOffCmd = async () => {
  const canExecute = coverC.value.calibrator.brightness != 0;
  const success = await sendCommand("/api/coverc/off", canExecute);
  if (success) {
    coverC.value.calibrator.brightness = 0;
  }
};

const calibratorBrightnessChange = async () => {
  if (
    coverC.value.calibrator.brightness < 0 ||
    coverC.value.calibrator.brightness > 4095
  ) {
    cmdRefusedNotify();
    return;
  }
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip + "/api/coverc/brightness", {
      method: "POST",
      headers: {
        "Content-Type": "application/x-www-form-urlencoded",
        Accept: "application/json, text/plain, */*",
      },
      body: new URLSearchParams({
        brightness: coverC.value.calibrator.brightness,
      }),
    });
    if (!response.ok) {
      throw new Error("Network response was not ok");
    }
    const res = await response.json();

    if (res.error) {
      errorResponseNotify(error);
      return;
    }
    if (res.execute) {
      cmdExecutedNotify();
      return;
    }
  } catch (error) {
    noResponseNotify(error);
  }
};

const cmdExecutedNotify = () => {
  toast.success(props.t("gen.cmdAck"), {
    autoClose: 500,
  });
};

const errorResponseNotify = (errorKey) => {
  const errorMessage = props.t(`errors.coverc.${errorKey}`);
  toast.error(errorMessage, {
    autoClose: 3000,
  });
};

const cmdRefusedNotify = () => {
  toast.error(props.t("gen.cmdRefused"), {
    autoClose: 500,
  });
};

const noResponseNotify = (error) => {
  toast.error(error, {
    autoClose: 3000,
  });
};

let intervalId = null;
onMounted(() => {
  startPolling();
});

onUnmounted(() => {
  stopPolling();
});
</script>
