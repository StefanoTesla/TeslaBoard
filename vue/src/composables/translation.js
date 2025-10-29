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

  /**
   * Accede in modo sicuro a proprietà nested nelle traduzioni
   * @param {string} path - Il path come "dome.home.roofState"
   * @param {string} fallback - Testo di fallback
   * @returns {string}
   */
  const t = (path, fallback = '⚠️ STRING_MISSING') => {
    if (!translations.value || !path) {
      if (import.meta.env.DEV) {
        console.warn(`Translation not loaded or empty path: ${path}`)
      }
      return fallback
    }
    
    const keys = path.split('.')
    let result = translations.value
    
    for (const key of keys) {
      if (result === null || result === undefined || !(key in result)) {
        if (import.meta.env.DEV) {
          console.warn(`🌐 Translation missing: ${path}`)
        }
        return fallback
      }
      result = result[key]
    }
    
    return result || fallback
  }

  return {
    translations,
    loadTranslations,
    t  // ← Esponi la funzione t
  };
}