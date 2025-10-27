import { ref } from "vue";

export function useTranslations() {
  const translations = ref({});

  const loadTranslations = async (locale) => {
    try {
      const response = await fetch(`/langs/${locale}.json`);
      const data = await response.json();
      translations.value = data;
    } catch (error) {
      console.error("Error loading translations:", error);
    }
  };

  return {
    translations,
    loadTranslations,
  };
}
