/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Nombre: input.ino
    
    Creado por:
      Copyright (c) Antonio Tamairón. 2023  / https://github.com/hash6iron/powadcr
      @hash6iron / https://powagames.itch.io/
    
    Descripción:
    Signal analyzer for REC function on powaDCR

    Version: 0.1

    Historico de versiones
    ----------------------
    v.0.1 - Version inicial
    
    Derechos de autor y distribución
    --------------------------------
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
    To Contact the dev team you can write to hash6iron@gmail.com
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
 

class TAPrecorder
{

    private:

      AudioKit _kit;

      const int BUFFER_SIZE = 2048;
      uint8_t* buffer;

      // Estamos suponiendo que el ZX Spectrum genera signal para grabar
      // con timming estandar entonces
      float tState = 1/3500000;
      int pilotPulse = 2168; // t-states
      int sync1 = 667;
      int sync2 = 735;
      int bit0 = 855;
      int bit1 = 1710;

      const int high = 32767;
      const int low = -32767;
      const int silence = 0;

      // Controlamos el estado del TAP
      int state = 0;
      // Contamos los pulsos del tono piloto
      int pilotPulseCount = 0;
      // Contamos los pulsos del sync1
      long silenceCount = 0;
      long lastSilenceCount = 0;
      bool isSilence = false;
      bool blockEnd = false;
      long silenceSample = 0;

      char* bitChStr;
      byte* datablock;
      byte* fileData;

      int bitCount = 0;
      int byteCount = 0;
      int blockCount = 0;
      String bitString = "";

      byte checksum = 0;
      byte lastChk = 0;
      byte byteRead = 0;

      // Estado flanco de la senal, HIGH or LOW
      bool pulseHigh = false;
      // Contador de bloques
      int bl = 0;
      // Umbral para rechazo de ruido
      int threshold = 20000; 
      //
      bool errorInSemiPulseDetection = false;
      //
      int pulseState = 0;
      int samplesCount_H = 0;
      int samplesCount_L = 0;

      struct tPulse
      {
        int high_edge;
        int low_edge;
      };

      tPulse pulse;

      struct tHeader
      {
          int type;
          char name[11];
          byte sizeL;
          byte sizeH;
      };

      tHeader header;



      // Calculamos las muestras que tiene cada pulso
      // segun el tiempo de muestreo
      // long getSamples(int signalTStates, float tState, int samplingRate)
      // {
      //     float tSampling = 1/samplingRate;
      //     float tPeriod = signalTStates * tState;
      //     long samples = tPeriod / tSampling;

      //     return samples; 
      // }

      byte calculateChecksum(byte* bBlock, int startByte, int numBytes)
            {
                // Calculamos el checksum de un bloque de bytes
                byte newChk = 0;

                // Calculamos el checksum (no se contabiliza el ultimo numero que es precisamente el checksum)
                
                for (int n=startByte;n<(startByte+numBytes);n++)
                {
                    newChk = newChk ^ bBlock[n];
                }

                return newChk;
            }

      int16_t prepareSignal(int16_t value, int threshold)
      {
          int16_t returnValue = 0;

          if (value > threshold)
          {
            returnValue = 32767;
          }
          else if (value < -threshold)
          {
              returnValue = -32767;
          }
          else
          {
            returnValue = 0;    
          }

        return returnValue;
      }

      bool detectedSilencePulse(int16_t value, int threshold)
      {
          if (value == 0 && silenceCount <= threshold)
          {
            // Si el pulso aun no ha completado un ancho y no ha habido errores
            // vamos bien.
            silenceCount++;
            return false;
          }
          else if (value == silence && silenceCount > threshold)
          {
              // Silencio detectado
              silenceCount = 0;
              return true;
          }
          else if (value != silence && silenceCount <= threshold)
          {
              // Silencio corrupto o menor del threshold
              silenceCount = 0;
              return false;
          }
          else if (value != silence && silenceCount > threshold)
          {
              // Silencio detectado
              silenceCount = 0;
              return true;
          }
      }

      bool measurePulse(int16_t value)
      {
          // Comenzamos por detectar un cambio de XX --> HIGH
          if (pulseState == 0 && value == high)
          {
              // Pasamos a modo medidas en HIGH
              // contamos ya esta muestra
              isSilence = false;
              silenceSample = 0;

      
              samplesCount_H=1;
              // Cambiamos de estado
              pulseState = 1;
              return false;
          }
          else if (pulseState == 0 && value == low)
          {
              // Error. Tengo que esperar al flanco positivo        
              pulseState = 0;
              silenceCount = 0;
              //Serial.println("Error. Waiting HIGH EDGE");
              return false;
          }
          else if (pulseState == 0 && value == 0)
          {
              // Error. Finalizo
              pulseState = 0;
              //silenceCount++;
              //Serial.println("Sequency cut. Error.");
              return false;
          }
          else if (pulseState == 1 && value == high)
          {
              // Estado de medida en HIGH
              silenceSample = 0;
              blockEnd = false;
              samplesCount_H++;
              return false;
          }
          else if (pulseState == 1 && value == low)
          {
              // Cambio de HIGH --> LOW
              // finaliza el pulso HIGH y comienza el LOW
              // cuento esta muestra
              samplesCount_L=1;
              // Paso al estado de medida en LOW
              pulseState = 2;
              return false;
          }
          else if (pulseState == 2 && value == low)
          {
              // Estado de medida en LOW
              samplesCount_L++;
              return false;
          }
          else if (pulseState == 2 && (value == high || value == 0))
          {
              // Ha finalizado el pulso. Pasa de LOW --> HIGH
              pulse.high_edge = samplesCount_H;
              pulse.low_edge = samplesCount_L;
              //Serial.println("PW: " + String(pulse.high_edge) + " - " + String(pulse.low_edge));

              if (value == 0)
              {
                blockEnd = true;
                // Reiniciamos HIGH
                samplesCount_H = 0;
                // Pongo a cero las muestras en LOW
                samplesCount_L = 0;
              }
              else
              {
                // Leemos esta muestra que es HIGH para no perderla
                samplesCount_H = 1;
                // Pongo a cero las muestras en LOW
                samplesCount_L = 0;
              }
              // Comenzamos otra vez
              pulseState = 1;        
              return true;
          }

          // Si llegamos aquí por un silencio es entonces fin de bloque
          if (value == 0 && blockEnd == true && silenceSample > 500)
          {
            isSilence = true;
            Serial.println("");
            Serial.println("** Silence active **");          
            Serial.println("");
            return true;
          }
          else
          {
            silenceSample++;
            return false;
          }
      }

      bool isGuidePulse(tPulse p)
      {
          if (p.high_edge >= 26 && p.high_edge <= 28)
          {
              if (p.low_edge >= 26 && p.low_edge <= 28)
              {
                  //
                  return true;
              }
              else
              {
                  return false;
              }
          }
          else
          {
              return false;
          }
      }

      bool isSync(tPulse p)
      {
          if (p.high_edge >= 7 && p.high_edge <= 10)
          {
              if (p.low_edge >= 7 && p.low_edge <= 10)
              {
                  //
                  return true;
              }
              else
              {
                  return false;
              }
          }
          else
          {
              return false;
          }
      }

      bool isBit0(tPulse p)
      {
          if (p.high_edge >= 10 && p.high_edge <= 11)
          {
              if (p.low_edge >= 10 && p.low_edge <= 11)
              {
                  //
                  return true;
              }
              else
              {
                  return false;
              }
          }
          else
          {
              return false;
          }
      }

      bool isBit1(tPulse p)
      {
          if (p.high_edge >= 20 && p.high_edge <= 22)
          {
              if (p.low_edge >= 20 && p.low_edge <= 22)
              {
                  //
                  return true;
              }
              else
              {
                  return false;
              }
          }
          else
          {
              return false;
          }
      }

      void getInfoByte()
      {
          // Almacenamos el tipo de cabecera
          if (byteCount == 2)
          {
              // 0 - program
              // 1 - Number array
              // 2 - Char array
              // 3 - Code file
              header.type = byteRead;
          }

          if (byteCount > 1 && byteCount < 12)
          {
              // Almacenamos el nombre
              if (byteRead > 20)
              {
                header.name[byteCount-2] = (char)byteRead;
              }
              else
              {
                int valueTmp = 32;
                header.name[byteCount-2] = (char)valueTmp;
              }
          }

          if (byteCount == 12)
          {
              // Almacenamos el LSB del tamaño
              header.sizeL = byteRead;
          }

          if (byteCount == 13)
          {
              // Almacenamos el MSB del tamaño
              header.sizeH = byteRead;
          }

      }

      void readBuffer(int len)
      {

        int16_t *value_ptr = (int16_t*) buffer;
        int16_t oneValue = *value_ptr++;
        int16_t oneValue2 = *value_ptr++;
        int16_t finalValue = 0;

          for (int j=0;j<len/4;j++)
          {  
              finalValue = prepareSignal(oneValue,threshold);
              //
              

                  if (measurePulse(finalValue))
                  {
                      // Ya se ha medido
                      if (state == 0  && pilotPulseCount <= 800)
                      {
                          if (isGuidePulse(pulse))
                          {
                              pilotPulseCount++;
                          }
                          else
                          {
                              pilotPulseCount = 0;
                          }
                      }
                      else if (state == 0 && pilotPulseCount > 800)
                      {
                          // Saltamos a la espera de SYNC1 y 2
                          state = 1;
                          pilotPulseCount = 0;
                          Serial.println("Wait for SYNC");
                      }
                      else if (state == 1 && isSync(pulse))
                      {
                          state = 2;
                          Serial.println("Wait for DATA");
                          Serial.println("");
                      }
                      else if (state == 2 && isBit0(pulse))
                      {
                          state = 2;
                          bitString += "0";
                          bitChStr[bitCount] = '0';
                          bitCount++;
                      }
                      else if (state == 2 && isBit1(pulse))
                      {
                          state = 2;
                          bitString += "1";
                          bitChStr[bitCount] = '1';
                          bitCount++;
                      }

                      if (bitCount > 7)
                      {
                        bitCount=0;
                        // Valor leido del DATA
                        byte value = strtol(bitChStr, (char**) NULL, 2);
                        byteRead = value;
                        lastChk = checksum;
                        //
                        checksum = checksum ^ value;

                        // Lo representamos
                        Serial.print(bitString + " - ");
                        Serial.print(value,HEX);
                        Serial.print(" - chk: ");
                        Serial.print(checksum,HEX);
                        Serial.println("");                    
                        
                        getInfoByte();                  

                        //datablock[byteCount] = value;
                        byteCount++;
                        bitString = "";
                      }
                  }

                  // Hay silencio, pero ¿Es silencio despues de bloque?
                  if (isSilence == true && blockEnd == true)
                  {
                    isSilence = false;
                    blockEnd = false;
                    blockCount++;

                    if (checksum == 0)
                    {
                      // Es el ultimo byte. Es el checksum
                      // si se hace XOR sobre este da 0
                      Serial.println("");
                      Serial.println("Last byte: CHK OK");                 
                    }            
                    Serial.println("");
                    Serial.print("Checksum: ");
                    Serial.print(lastChk,HEX);
                    Serial.println("");
                    Serial.println("Block read: " + String(blockCount));
                    checksum = 0;
                    state = 0;


                    if (byteCount !=0)
                    {
                        Serial.println("");
                        Serial.println("Block data was recorded successful");
                        Serial.println("");
                        Serial.println("Total bytes: " + String(byteCount));
                        Serial.println("");

                        // Si el conjunto leido es de 19 bytes. Es una cabecera.
                        if (byteCount == 19)
                        {
                            int size = 0;
                            size = header.sizeL + header.sizeH*256;

                            //redimensionamos
                            fileData = (byte*)realloc(fileData,25+size+2);

                            Serial.print("PROGRAM: ");

                            for (int n=0;n<10;n++)
                            {
                              Serial.print(header.name[n]);
                            }
                            Serial.println("");
                            Serial.println("Block size: " + String(size) + " bytes");
                            Serial.println("");
                            Serial.println("");
                        }
                        else if (byteCount > 19)
                        {
                            if (byteCount ==  6914)
                            {
                              Serial.println("SCREEN");
                            }
                            else
                            {
                              Serial.println("DATA");
                            }

              
                            // Guardamos ahora en fichero

                            // Liberamos
                            free(fileData);
                        }
                    }
                    else
                    {
                        Serial.println("");
                        Serial.println("Error in block data recording process!");
                        Serial.println("");
                    }

                    byteCount = 0;
                    
                  }

              // Ahora medimos el pulso detectado

              // Leemos el siguiente valor
              oneValue = *value_ptr++;
              // Este solo es para desecharlo
              oneValue2 = *value_ptr++;        
          }
      }

    public:

      void set_kit(AudioKit kit)
      {
        _kit = kit;
      }

      void recording()
      {
          size_t len = _kit.read(buffer, BUFFER_SIZE);
          readBuffer(len);        
      }

      void initialize()
      {
          // Inicializo bit string
          bitChStr = (char*)calloc(8, sizeof(char));
          datablock = (byte*)calloc(1, sizeof(byte));
          
          // Inicializamos el array de nombre del header
          for (int i=0;i<10;i++)
          {
            header.name[i] = ' ';
          }

          fileData = (byte*)calloc(25,sizeof(byte));  
          buffer = (uint8_t*)calloc(BUFFER_SIZE,sizeof(uint8_t));
      }

      void terminate()
      {
          free(buffer);
          free(datablock);
          free(bitChStr);
      }

      TAPrecorder()
      {}
};