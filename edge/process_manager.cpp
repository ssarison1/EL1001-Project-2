#include "process_manager.h"
#include "opcode.h"
#include "byte_op.h"
#include "setting.h"
#include <cstring>
#include <iostream>
#include <ctime>
using namespace std;

ProcessManager::ProcessManager()
{
  this->num = 0;
}

void ProcessManager::init()
{
}

// TODO: You should implement this function if you want to change the result of the aggregation
/*uint8_t *ProcessManager::processData(DataSet *ds, int *dlen)
{
  uint8_t *ret, *p;
  int num, len;
  HouseData *house;
  Info *info;
  TemperatureData *tdata;
  HumidityData *hdata;
  PowerData *pdata;
  char buf[BUFLEN];
  ret = (uint8_t *)malloc(BUFLEN);
  int tmp, min_humid, min_temp, min_power, month;
  time_t ts;
  struct tm *tm;

  tdata = ds->getTemperatureData();
  hdata = ds->getHumidityData();
  num = ds->getNumHouseData();

  // Example) I will give the minimum daily temperature (1 byte), the minimum daily humidity (1 byte), 
  // the minimum power data (2 bytes), the month value (1 byte) to the network manager
  
  // Example) getting the minimum daily temperature
  min_temp = (int) tdata->getMin();

  // Example) getting the minimum daily humidity
  min_humid = (int) hdata->getMin();

  // Example) getting the minimum power value
  min_power = 10000;
  for (int i=0; i<num; i++)
  {
    house = ds->getHouseData(i);
    pdata = house->getPowerData();
    tmp = (int)pdata->getValue();

    if (tmp < min_power)
      min_power = tmp;
  }

  // Example) getting the month value from the timestamp
  ts = ds->getTimestamp();
  tm = localtime(&ts);
  month = tm->tm_mon + 1;

  // Example) initializing the memory to send to the network manager
  memset(ret, 0, BUFLEN);
  *dlen = 0;
  p = ret;

  // Example) saving the values in the memory
  VAR_TO_MEM_1BYTE_BIG_ENDIAN(min_temp, p);
  *dlen += 1;
  VAR_TO_MEM_1BYTE_BIG_ENDIAN(min_humid, p);
  *dlen += 1;
  VAR_TO_MEM_2BYTES_BIG_ENDIAN(min_power, p);
  *dlen += 2;
  VAR_TO_MEM_1BYTE_BIG_ENDIAN(month, p);
  *dlen += 1;

  return ret;
}*/
uint8_t *ProcessManager::processData(DataSet *ds, int *dlen)
{
    uint8_t *ret, *p;
    int num;
    HouseData *house;
    TemperatureData *tdata;
    HumidityData *hdata;
    PowerData *pdata;

    ret = (uint8_t *)malloc(BUFLEN);
    memset(ret, 0, BUFLEN);

    time_t ts;
    struct tm *tm;

    tdata = ds->getTemperatureData();
    hdata = ds->getHumidityData();
    num = ds->getNumHouseData();

    // 1. 실수 데이터 추출 및 정수 스케일링 (x 1)
    // getValue() 등의 메서드로 ^평균값^을 가져온다.
    int scaled_temp = (int)(tdata->getValue() * 1.0);
    int scaled_humid = (int)(hdata->getValue() * 1.0);

    // 2. 전력 데이터 추출 (예: 모든 집의 ^평균^ 전력 사용량)
    int total_power = 0;
    for (int i = 0; i < num; i++)
    {
        house = ds->getHouseData(i);
        pdata = house->getPowerData();
        total_power += (int)pdata->getValue();
    }
    int avg_power = (num > 0) ? (total_power / num) : 0;

    // 3. Timestamp에서 Month 추출
    ts = ds->getTimestamp();
    tm = localtime(&ts);
    int month = tm->tm_mon + 1;

    // Example) initializing the memory to send to the network manager
    memset(ret, 0, BUFLEN);
    *dlen = 0;
    p = ret;

    // Example) saving the values in the memory
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(scaled_temp, p);
    *dlen += 1;
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(scaled_humid, p);
    *dlen += 1;
    VAR_TO_MEM_2BYTES_BIG_ENDIAN(avg_power, p);
    *dlen += 2;
    VAR_TO_MEM_1BYTE_BIG_ENDIAN(month, p);
    *dlen += 1;

    return ret;
}
