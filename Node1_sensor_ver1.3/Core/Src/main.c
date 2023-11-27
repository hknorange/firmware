/*
 * code note
 * 250923
 * edit 051023
 * STM32F108C8T6
 * BLE -> UART3 -> 9600
 * fuel -> UART2 -> 9600
 * LED SIGN1 -> PB13
 * LED SIGN2 -> PB12
 * WEIGHT -> ADC A0
 */
#include "main.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <stdbool.h>
#include <ctype.h>
#include "flash.h"
#define TIMESEND 8000

ADC_HandleTypeDef hadc1;
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_IWDG_Init(void);

/*-------------- create sketch-----------------------------------*/
//FLASH SAVE VOLUME FUEL
#define DATA_VOLUME_ADDRESS ((uint32_t)0x0801F810) //PAGE 126
#define LEN_VOLUME_ADDRESS	((uint32_t)0x0801FC00) //PAGE 127
//FLASH SAVE HIGH FUEL
#define DATA_HIGH_ADDRESS 	((uint32_t)0x0801F000)  //page 124
#define LEN_HIGH_ADDRESS	((uint32_t)0x0801F400)  //page 125
//FLASH SAVE WEIGHT
#define DATA_WEIGHT_ADDRESS	((uint32_t)0x0801E800)	 //page  122
#define LEN_WEIGHT_ADDRESS	((uint32_t)0x0801EC00)	 //page 123
//FLASH SAVE CONCER
#define DATA_CONCER_ADDRESS	((uint32_t)0x0801E000)  //page 120
#define LEN_CONCER_ADDRESS	((uint32_t)0x0801E400)  //page 121

/*
 * struct data type
 */
typedef struct{
	uint16_t weight_value_adc[6];
	float weight_voltage;
	float weight;
	uint8_t cout;
}ADCS;

typedef struct{
	uint8_t rxdata[128];
	uint16_t oil_level_value;
	uint8_t rxbuffer;
	uint8_t index;
	uint16_t num_level;
	float flue;
	bool flag;
}Flue;

typedef struct{
	uint8_t rx_array_buffer[30];
//	uint8_t tx_buffer[128];
	uint8_t rx_buffer;
	uint8_t index;
}Ble;

ADCS adc_weight = {{'\0'},0,0,0};
Flue fuelchina;
Ble bluetooth = {{'\0'},0,0};

uint32_t timeout_tick;
uint8_t data_send[20];
//const edit
uint16_t MAX_ADC = 4095;
float MAX_VOLTAGE = 3.26;
float MAX_VOL = 4; //volume tank oil
float MAX_HEIGH = 235;//oil tank height UNIT MM
float MAX_WEIGH = 60000; //UNIT KG
float MAX_CONCER = 120;
bool sta_flash = false;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
/*
 *funtion for adc
 */
void ADC_Read_Sensor();
void ADC_Channel_Select(uint32_t channel);
void ADC_Weight(float val);
float ADC_AVR_Weight(uint16_t arr[]);
/*
 * funtion for flue china
 */
char *UART_Get_Fuel(void);
void Fuel_China(void);
char *substring(char *string, int position, int length);
bool is_float_number(const char* str);
/*
 * function infor value const
 */
void read_infor_default(void);
void update_infor_default(char *rec);
void write_infor_default(void);
void check_connect_ble(char* er);
/*
 * interrupt timer 2
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	UNUSED(htim);
//	HAL_GPIO_TogglePin(LED_SIGN1_GPIO_Port, LED_SIGN1_Pin);
	HAL_GPIO_TogglePin(LED_SIGN2_GPIO_Port, LED_SIGN2_Pin);
	ADC_Read_Sensor();
//	printf("----------interrupt for ADC-------\r\n");
}

int main(void)
{

	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_ADC1_Init();
	MX_USART1_UART_Init();
	MX_TIM2_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_IWDG_Init();
	HAL_TIM_Base_Start_IT(&htim2);
	printf("@@@@@@@@@@@@@@@@@@@@@\r\n");
	//	write_infor_default();
	read_infor_default();
	timeout_tick = HAL_GetTick();
	HAL_UART_Receive_IT(&huart3,&bluetooth.rx_buffer,1);
	while (1)
	{
		Fuel_China();
		if(HAL_GetTick() - timeout_tick > TIMESEND){
		  sprintf((char*)data_send,"\"%.2f\",\"%.2f\"",fuelchina.flue,adc_weight.weight);
	//		  printf("len:%d\r\n",strlen(data_send));
	//		  printf("len:%d\r\n",sizeof(data_send));
		  HAL_UART_Transmit(&huart3, data_send, strlen(data_send), 300);
		  printf("data:%s\r\n",data_send);
		  timeout_tick = HAL_GetTick();
		}
		if(sta_flash == true){
	//			printf("############\r\n");
	//			printf("%s\r\n",(char*)bluetooth.rx_array_buffer);
			update_infor_default((char*)bluetooth.rx_array_buffer);
			sta_flash = false;


		}
		check_connect_ble((char*)bluetooth.rx_array_buffer);
//		printf("bluetooth.rx_array_buffer:%s\r\n",bluetooth.rx_array_buffer);
//		HAL_Delay(20000);
		HAL_IWDG_Refresh(&hiwdg);
	}

}

/*
 * interrup uart ble
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	printf("aaaaaaaaaaaa\r\n");
	if(huart->Instance == huart3.Instance){
		if(bluetooth.rx_buffer != '#'){
			bluetooth.rx_array_buffer[bluetooth.index] = bluetooth.rx_buffer;
			bluetooth.index++;
//			printf("bbbbbbbbbbbbbbbbb\r\n");
			//HAL_UART_Transmit(&huart1, &bluetooth.rx_buffer, 1, 1000);
		}
		else{
			bluetooth.rx_array_buffer[bluetooth.index] = '\0';
			bluetooth.index = 0;
			printf("data ble:%s\n",bluetooth.rx_array_buffer);
			sta_flash = true;
		}

		HAL_UART_Receive_IT(&huart3, &bluetooth.rx_buffer, 1);
	}
}

/*
 * read value adc
 */

void ADC_Read_Sensor(){
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	++adc_weight.cout;
	adc_weight.weight_value_adc[adc_weight.cout] = HAL_ADC_GetValue(&hadc1);
//	printf("adc value %d:%d\r\n",adc_weight.cout,adc_weight.weight_value_adc[adc_weight.cout]);
	HAL_ADC_Stop(&hadc1);
	if(adc_weight.cout == 5){
		adc_weight.cout = 0;
		adc_weight.weight_value_adc[0] = ADC_AVR_Weight(adc_weight.weight_value_adc);
		ADC_Weight(adc_weight.weight_value_adc[0]);
	}
}

/*
 *choose channel adc
 */
void ADC_Channel_Select(uint32_t channel){
	ADC_ChannelConfTypeDef conf = {
			.Channel = channel,
			.Rank = 1,
			.SamplingTime = ADC_SAMPLETIME_1CYCLE_5,
	};
	if(HAL_ADC_ConfigChannel(&hadc1, &conf) != HAL_OK){
		Error_Handler();
	}
}

/*
 * adc cal aver value adc
 */
float ADC_AVR_Weight(uint16_t arr[]){
	float result;
	for(int i=1;i<=5;i++)
		result += arr[i];

	return result/5.0;
}
/*
 * get value weight
 */
void ADC_Weight(float val){
	float weight_voltage = (float)val/3971*3.2;
//	printf("voltage weight:%f",weight_voltage);
	adc_weight.weight = ((weight_voltage *MAX_CONCER /3.2)*MAX_WEIGH)/MAX_CONCER;
}
/*
 * get value uart fuel china
 * *XD,0000,01,2170,2171,0000,0276,1357#
 *
 */
char* UART_Get_Fuel(void){
	char *level_value;
	char *pf;
	uint8_t lenstr = 0;
	HAL_UART_Receive(&huart2, fuelchina.rxdata, sizeof(fuelchina.rxdata), 1000);
	pf = strstr((char*)fuelchina.rxdata, "XD");
	if(pf != NULL){
	lenstr = strcspn((char*)fuelchina.rxdata, "XD");
//	printf("index:%d\r\n",lenstr);
//	printf("fuel:%s\r\n",(char*)fuelchina.rxdata);
	level_value = substring((char*)fuelchina.rxdata,lenstr + 17,4);
//	printf("fuel number:%s\r\n",level_value);
	return level_value;
	}
	else{
		return 0;
	}
}
/*
 * GET VALUE FUEL china
 */
void Fuel_China(void){
	char *result;
	result = UART_Get_Fuel();
//	printf("FUel china:%f\r\n",atoi(result)/10.0);
	fuelchina.flue = (atoi(result)/10.0)* MAX_VOL/(MAX_HEIGH*1000);
}
/*
 * substring string
 */

char *substring(char *string, int position, int length)
{
   char *p;
   int c;
   p = malloc(length+1);
   if (p == NULL){
      printf("Unable to allocate memory.\n");
      exit(1);
   }
   for (c = 0; c < length; c++)
   {
      *(p+c) = *(string+position-1);
      string++;
   }
   *(p+c) = '\0';
   return p;
}
/*
 * read value in fash
 */
void read_infor_default(void){
	printf("------------------readinfordefault########\r\n");
	char data[10];
	Flash_ReadChar(data, DATA_VOLUME_ADDRESS, LEN_VOLUME_ADDRESS);
//	printf("data:%s\r\n",data);
	MAX_VOL = atof(data);
	printf("VOL:%f\r\n",MAX_VOL);
	memset(data,'\0',10);
	HAL_Delay(300);
	Flash_ReadChar(data, DATA_WEIGHT_ADDRESS, LEN_WEIGHT_ADDRESS);
//	printf("data:%s\r\n",data);
	MAX_WEIGH = atof(data);
	printf("WEIGHT:%f\r\n",MAX_WEIGH);
	memset(data,'\0',10);
	HAL_Delay(300);
	Flash_ReadChar(data, DATA_HIGH_ADDRESS, LEN_HIGH_ADDRESS);
//	printf("data:%s\r\n",data);
	MAX_HEIGH = atof(data);
	printf("HEIGH:%f\r\n",MAX_HEIGH);
	memset(data,'\0',10);
	HAL_Delay(300);
//	Flash_ReadChar(data, DATA_CONCER_ADDRESS, LEN_CONCER_ADDRESS);
//	printf("data:%s\r\n",data);
//	MAX_CONCER = atof(data);
//	printf("CONCER:%f\r\n",MAX_CONCER);
//	memset(data,'\0',10);
//	HAL_Delay(300);
}
void write_infor_default(void){
	Flash_ProgramPage("4", DATA_VOLUME_ADDRESS, LEN_VOLUME_ADDRESS);
	HAL_Delay(150);
	Flash_ProgramPage("60000", DATA_WEIGHT_ADDRESS, LEN_WEIGHT_ADDRESS);
	HAL_Delay(150);
	Flash_ProgramPage("235", DATA_HIGH_ADDRESS, LEN_HIGH_ADDRESS);
	HAL_Delay(150);
//	Flash_ProgramPage("120", DATA_CONCER_ADDRESS, LEN_CONCER_ADDRESS);
//	HAL_Delay(150);

}
bool is_float_number(const char* str){
	char *p;
	float ret;
	ret = strtod(str,&p);
//	printf("@@@@@@@@@@@\r\n");
//	printf("Phan gia tri (double) la: %lf\r\n", ret);
	if(strlen(p) >= 1){
		return false;
	}
	else{
		return true;
	}
}
void update_infor_default(char *rec){ //400.50,1.19,45.5# //@400.0,1.19,60000.0#
	bool act = true;
//	printf("bluetooth:%s\r\n",rec);
	if(strcmp(rec,"n/a") == 0){
		printf("string la n/a\r\n");
		return;
	}
//	printf("bbbbbbbbbbbbbbbbbbbbbbb\r\n");
	char *sub = strtok(rec,",");
	char data[4][10];
	uint8_t index=0;
	while(sub != NULL){
//		printf("%s\r\n",sub);q
		strcpy(data[index],sub);
//		if(!is_float_number(data[index])){
//			act = false;
			printf("float number %d:%d\n",index,is_float_number(data[index]));
//		}

		sub = strtok(NULL,",");
		index ++;
		printf("so number index %d\n",index);

	}
	if(index == 3){
//		printf("?????????????\n");
//		printf("000000000000\n");
//		printf("%s\n",data[0]);
//		printf("%s\n",data[1]);
//		printf("%s\n",data[2]);

		Flash_ProgramPage(data[0], DATA_VOLUME_ADDRESS, LEN_VOLUME_ADDRESS);
		HAL_Delay(40);
		Flash_ProgramPage(data[2], DATA_WEIGHT_ADDRESS, LEN_WEIGHT_ADDRESS);
		HAL_Delay(40);
		Flash_ProgramPage(data[1], DATA_HIGH_ADDRESS, LEN_HIGH_ADDRESS);
		HAL_Delay(40);
		memset(bluetooth.rx_array_buffer,'\0',30);
		NVIC_SystemReset();
	}
	else{
//		printf("index khong bang 3\n");
	}

//	printf("ket thuc update ble\r\n");


}
void check_connect_ble(char *er){
	char *err;
	err = strstr(er,"ERROR");
	if(err!=NULL){
		memset(bluetooth.rx_array_buffer,'\0',30);
	}
}





#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_SIGN2_Pin|LED_SIGN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_SIGN2_Pin LED_SIGN1_Pin */
  GPIO_InitStruct.Pin = LED_SIGN2_Pin|LED_SIGN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
