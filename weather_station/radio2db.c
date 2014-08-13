#include <stdio.h>
#include <string.h>
#include "gpio_lib.h"
#include "nRF24L01P.h"
#include <sqlite3.h>

typedef struct{
  int           location;               /* Numero stanza e numero sensore */
  int           pressure;               /* Dati letti dal sensore BMP180 */
  float         altitude;
  float         temperature;
  float         humidity;               /* Dati letti dal sensore DHT */
  float         temperature2;
} ENVDATA;

ENVDATA envData;
unsigned char rxData[ sizeof( envData)];
int rxDataCnt;

/* SQLITE handle */
sqlite3 *db;

/* SQLITE function declaretion */
int open_database( char *dbname);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);

/* SQLITE callback function */
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      fprintf( stderr, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   fprintf( stderr, "\n");
   return 0;
}


int main (int argc, char *argv[])
{
  /* SQLITE variable */
  char *zErrMsg = 0;
  int  rc;
  char *sql;
  char sql_insert[1024];

  /* */
  nRF24L01P_nRF24L01P();
  nRF24L01P_flush();
  nRF24L01P_powerUp();

  nRF24L01P_setRfFrequency ( 2500);
  nRF24L01P_setTransferSize( sizeof( envData), NRF24L01P_PIPE_P0);
  nRF24L01P_setCrcWidth(8);
  nRF24L01P_setAirDataRate( NRF24L01P_DATARATE_250_KBPS);
  nRF24L01P_setRfOutputPower( NRF24L01P_TX_PWR_ZERO_DB);
  nRF24L01P_enableAutoAcknowledge( NRF24L01P_PIPE_P0);
  nRF24L01P_setRxAddress( 0x00F0F0F0F0LL, DEFAULT_NRF24L01P_ADDRESS_WIDTH, NRF24L01P_PIPE_P0);
  nRF24L01P_setTxAddress( 0xF0F0F0F0D2LL, DEFAULT_NRF24L01P_ADDRESS_WIDTH);

  nRF24L01P_setReceiveMode();
  nRF24L01P_enable();

  fprintf( stderr, "RADIO SETUP DONE\n");
  // Display the setup of the nRF24L01+ chip
  fprintf( stderr, "nRF24L01+ Frequency    : %d MHz\n",  nRF24L01P_getRfFrequency() );
  fprintf( stderr, "nRF24L01+ Data Rate    : %d kbps\n", nRF24L01P_getAirDataRate() );
  fprintf( stderr, "nRF24L01+ TX Address   : 0x%010llX\n", nRF24L01P_getTxAddress() );
  fprintf( stderr, "nRF24L01+ RX Address   : 0x%010llX\n", nRF24L01P_getRxAddress( NRF24L01P_PIPE_P0) );
  fprintf( stderr, "nRF24L01+ CrC Width    : %d CrC\n", nRF24L01P_getCrcWidth() );
  fprintf( stderr, "nRF24L01+ TransferSize : %d Paket Size\n", nRF24L01P_getTransferSize( NRF24L01P_PIPE_P0) );
  fprintf( stderr, "RADIO SETUP DONE\n");
  fflush(stderr);

  while (1) {
        if ( nRF24L01P_readable( NRF24L01P_PIPE_P0) ) {
		rxDataCnt = nRF24L01P_read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
		ENVDATA *env_ptr = (ENVDATA*)rxData; // pointer manipulation split out to make working obvious
		envData = *env_ptr;
		/* */
		fflush(stdout);
		fprintf( stdout, "%d;%d;%f;%f;%f;%f\r\n", envData.location,
							envData.pressure,
							envData.altitude,
							envData.temperature,
							envData.humidity,
							envData.temperature2 );
		fflush(stdout);
		fflush(stdout);
		fflush(stdout);
		fflush(stdout);
		/* */
		if ( argc > 1 && open_database( argv[1])) {
			/* Create SQL statement */
			sql = "CREATE TABLE IF NOT EXISTS samples (id INTEGER PRIMARY KEY AUTOINCREMENT," \
								  "date TEXT," \
								  "time TEXT," \
								  "location INTEGER," \
								  "pressure INTEGER," \
								  "altitude REAL," \
								  "temperature REAL," \
								  "humidity REAL," \
								  "temperature2 REAL);";

			/* Execute SQL statement */
			rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
			if( rc != SQLITE_OK ){
				fprintf( stderr, "SQL error: %s\n", zErrMsg);
				sqlite3_free(zErrMsg);
			}else {
				fprintf( stderr, "Table created successfully\n");
				/* */
				sprintf( sql_insert, "INSERT INTO samples VALUES (null, date('now','localtime'),\
								  time('now','localtime'),\
								  %d, %d, %f, %f, %f, %f);", envData.location,\
								  envData.pressure,\
								  envData.altitude,\
								  envData.temperature,\
								  envData.humidity,\
								  envData.temperature2 );
				/* Execute SQL statement */
				rc = sqlite3_exec(db, sql_insert, callback, 0, &zErrMsg);
				if( rc != SQLITE_OK ){
					fprintf( stderr, "SQL error: %s\n", zErrMsg);
					sqlite3_free(zErrMsg);
				}else{
					fprintf( stderr, "Records created successfully\n");
				}
				sqlite3_close(db);
			}
		}
		/* */
		usleep( 100);
       	}
  }

}

int open_database( char *dbname)
{
	int  rc;
	/* Open database */
	rc = sqlite3_open(dbname, &db);
	if( rc ){
		fprintf( stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return( 0);
	}else{
		fprintf( stderr, "Opened database successfully\n");
		return( 1);
	}
}

