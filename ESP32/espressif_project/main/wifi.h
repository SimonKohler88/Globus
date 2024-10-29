/*
 * wifi.h
 *
 *  Created on: 1 May 2021
 *      Author: cyril
 *
 *  copied and adapted: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

void wifi_receive_init(void);
void wifi_receive_udp_task(void *pvParameters);

#endif /* MAIN_WIFI_H_ */
