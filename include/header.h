
/* ALPACA DATA */

struct AlpacaCommonData{
  uint32_t serverTransactionID = 0;
};

AlpacaCommonData AlpacaData;

/*END ALPACA DATA */

static char rxBuffer[96];
static size_t rxIndex = 0;
static bool rxInProgress = false;
