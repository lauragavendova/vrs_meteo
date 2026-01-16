# vrs_meteo

Meterologická stanica s predpoveďou počasia

Hardvér:
- STM32F303K8TX
- ILI9341TFT
- IKS01A1

Popis zadania:
MCU komunikuje so senzormi cez I2C a získava údaje o aktuálnej teplote a tlaku. Na základe teploty a tlaku vyhodnotí aké je počasie.
Na displeji, ktorý komunikuje s MCU cez SPI sa zobrazujú namerané hodnoty a opis počasia textový, grafický. 
Na displeji sa tiež zobrazuje dátum a čas, ktorý sa nastavuje cez PC. PC komunikuje s MCU cez UART. 
Spínačom je možné prepínať jednotlivé obrazovky grafického rozhrania. Obrazovky je možné prepínať aj cez PC.

Úlohy a rozdelenie:
- čítanie, formátovanie a spracovanie dát zo senzorov cez I2C - (Sebastian Bodor)
- ovládanie a nastavovanie aplikácie cez PC komunikácia cez USART - (Tadeáš Kapina)
- komunikácia s displejom cez SPI, konfigurácia a hardvér - (Tomáš Krajmer)
- grafické rozhranie, vykreslovanie údajov a objektov - (Laura Gavendová)

