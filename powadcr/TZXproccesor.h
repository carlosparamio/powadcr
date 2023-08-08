/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Nombre: TZXproccesor.h
    
    Creado por:
      Copyright (c) Antonio Tamairón. 2023  / https://github.com/hash6iron/powadcr
      @hash6iron / https://powagames.itch.io/
    
    Descripción:
    Conjunto de recursos para la gestión de ficheros .TZX del ZX Spectrum

    Version: 1.0

    Historico de versiones

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

class TZXproccesor
{

    private:

    HMI _hmi;

    // Definicion de un TZX
    SdFat32 _sdf32;
    tTZX _myTZX;
    File32 _mFile;
    int _sizeTZX;
    int _rlen;

    int CURRENT_LOADING_BLOCK = 0;

     byte calculateChecksum(byte* bBlock, int startByte, int numBytes)
     {
         // Calculamos el checksum de un bloque de bytes
         byte newChk = 0;

         #if LOG>3
           Serial.println("");
           Serial.println("Block len: ");
           Serial.print(sizeof(bBlock)/sizeof(byte*));
         #endif

         // Calculamos el checksum (no se contabiliza el ultimo numero que es precisamente el checksum)
         
         for (int n=startByte;n<(startByte+numBytes);n++)
         {
             newChk = newChk ^ bBlock[n];
         }

         #if LOG>3
           Serial.println("");
           Serial.println("Checksum: " + String(newChk));
         #endif

         return newChk;
     }      

      // char* getNameFromHeader(byte* header, int startByte)
      // {
      //     // Obtenemos el nombre del bloque cabecera
      //     char* prgName = (char*)calloc(10+1,sizeof(char));

      //     if (isCorrectHeader(header,startByte))
      //     {
      //         // Es una cabecera PROGRAM 
      //         // el nombre está en los bytes del 4 al 13 (empezando en 0)
      //         #if LOG==3
      //           Serial.println("");
      //           Serial.println("Name detected ");
      //         #endif

      //         // Extraemos el nombre del programa
      //         for (int n=4;n<14;n++)
      //         {   
      //             prgName[n-4] = (char)header[n];
      //         }
      //     }
      //     else
      //     {
      //         prgName = &INITCHAR[0];
      //     }

      //     // Pasamos la cadena de caracteres
      //     return prgName;

      // }

     byte* getBlockRange(byte* bBlock, int byteStart, int byteEnd)
     {

         // Extraemos un bloque de bytes indicando el byte inicio y final
         //byte* blockExtracted = new byte[(byteEnd - byteStart)+1];
         byte* blockExtracted = (byte*)calloc((byteEnd - byteStart)+1, sizeof(byte));

         int i=0;
         for (int n=byteStart;n<(byteStart + (byteEnd - byteStart)+1);n++)
         {
             blockExtracted[i] = bBlock[n];
         }

         return blockExtracted;
     }

     bool isHeaderTZX(File32 tzxFile)
     {
        if (tzxFile != 0)
        {

           Serial.println("");
           Serial.println("Begin isHeaderTZX");

           // Capturamos la cabecera

           //byte* bBlock = NULL; 
           byte* bBlock = (byte*)calloc(10+1,sizeof(byte));
           bBlock = sdm.readFileRange32(tzxFile,0,10,false);

          // Obtenemos la firma del TZX
          char* signTZXHeader = (char*)calloc(8+1,sizeof(char));

          // Analizamos la cabecera
          // Extraemos el nombre del programa
           for (int n=0;n<7;n++)
           {   
               signTZXHeader[n] = (char)bBlock[n];
           }
           
           //Aplicamos un terminador a la cadena de char
           signTZXHeader[7] = '\0';
           //Convertimos a String
           String signStr = String(signTZXHeader);
           
           Serial.println("");
           Serial.println("Sign: " + signStr);

           if (signStr.indexOf("ZXTape!") != -1)
           {
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

     bool isFileTZX(File32 tzxFile)
     {
         if (tzxFile != 0)
         {

            // Capturamos el nombre del fichero en szName
            char* szName = (char*)calloc(255,sizeof(char));
            tzxFile.getName(szName,254);
            
            //String szNameStr = sdm.getFileName(tzxFileName);
            String fileName = String(szName);

            Serial.println("");
            Serial.println("Name " + fileName);

            if (fileName != "")
            {
                  //String fileExtension = szNameStr.substring(szNameStr.length()-3);
                  fileName.toUpperCase();
                  
                  if (fileName.indexOf(".TZX") != -1)
                  {
                      //La extensión es TZX pero ahora vamos a comprobar si internamente también lo es
                      if (isHeaderTZX(tzxFile))
                      { 
                        //Cerramos el fichero porque no se usará mas.
                        tzxFile.close();
                        return true;
                      }
                      else
                      {
                        //Cerramos el fichero porque no se usará mas.
                        tzxFile.close();
                        return false;
                      }
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
         else
         {
            return false;
         }
  
     }

    int getDWORD(File32 mFile, int offset)
    {
        int sizeDW = 0;
        sizeDW = (256*sdm.readFileRange32(mFile,offset+1,1,false)[0]) + sdm.readFileRange32(mFile,offset,1,false)[0];  

        return sizeDW;    
    }

    int getBYTE(File32 mFile, int offset)
    {
        int sizeB = 0;
        sizeB = sdm.readFileRange32(mFile,offset,1,false)[0];  

        return sizeB;      
    }

    int getNBYTE(File32 mFile, int offset, int n)
    {
        int sizeNB = 0;

        Serial.println("");
        Serial.println("HEX VALUE n bytes");

        for (int i = 0; i<n;i++)
        {
            sizeNB += pow(2,(8*i)) * (sdm.readFileRange32(mFile,offset+i,1,false)[0]);  
        }

        Serial.println("");
        Serial.println("DECIMAL");
        Serial.println(String(sizeNB));

        return sizeNB;           
    }

    int getID(File32 mFile, int offset)
    {
        // Obtenemos el ID
        int ID = 0;
        ID = getBYTE(mFile,offset);

        return ID;      
    }


    byte* getBlock(File32 mFile, int offset, int size)
    {
        //Entonces recorremos el TZX. 
        // La primera cabecera SIEMPRE debe darse.
        byte* bloque = (byte*)calloc(size+1,sizeof(byte));
        // Obtenemos el bloque
        if (bloque != NULL)
        {
            bloque = sdm.readFileRange32(mFile,offset,size,false);          
        }

        return bloque;
    }

    bool verifyChecksum(File32 mFile, int offset, int size)
    {
        // Vamos a verificar que el bloque cumple con el checksum
        // Cogemos el checksum del bloque
        byte chk = getBYTE(mFile,offset+size-1);
        Serial.println("");
        Serial.println("Original checksum:");
        Serial.print(chk,HEX);

        byte* block = getBlock(mFile,offset,size-1);
        byte calcChk = calculateChecksum(block,0,size-1);
        Serial.println("");
        Serial.println("Calculated checksum:");
        Serial.print(calcChk,HEX);
        Serial.println("");

        if (chk == calcChk)
        {
            return true;
        }
        else
        {            
          return false;
        }
    }

    void analyzeID16(File32 mFile, int currentOffset, int currentBlock)
    {
      //Normal speed block

      //
      // TAP description block
      //
      
      //       |------ Spectrum-generated data -------|       |---------|

      // 13 00 00 03 52 4f 4d 7x20 02 00 00 00 00 80 f1 04 00 ff f3 af a3

      // ^^^^^...... first block is 19 bytes (17 bytes+flag+checksum)
      //       ^^... flag byte (A reg, 00 for headers, ff for data blocks)
      //          ^^ first byte of header, indicating a code block

      // file name ..^^^^^^^^^^^^^
      // header info ..............^^^^^^^^^^^^^^^^^
      // checksum of header .........................^^
      // length of second block ........................^^^^^
      // flag byte ...........................................^^
      // first two bytes of rom .................................^^^^^
      // checksum (checkbittoggle would be a better name!).............^^      
     
        int headerTAPsize = 21;   //Contando blocksize + flagbyte + etc hasta 
                                  //checksum
        int dataTAPsize = 0;
        
        tTZXBlockDescriptor blockDescriptor; 
        _myTZX.descriptor[currentBlock].ID = 16;
        _myTZX.descriptor[currentBlock].playeable = true;
        _myTZX.descriptor[currentBlock].offset = currentOffset;


        //SYNC1
        // zxp.SYNC1 = DSYNC1;
        // //SYNC2
        // zxp.SYNC2 = DSYNC2;
        // //PULSE PILOT
        // zxp.PULSE_PILOT = DPULSE_PILOT;                              
        // // BTI 0
        // zxp.BIT_0 = DBIT_0;
        // // BIT1                                          
        // zxp.BIT_1 = DBIT_1;
        // No contamos el ID. Entonces:
        
        // Timming de la ROM
        _myTZX.descriptor[currentBlock].timming.pulse_pilot = DPULSE_PILOT;
        _myTZX.descriptor[currentBlock].timming.sync_1 = DSYNC1;
        _myTZX.descriptor[currentBlock].timming.sync_2 = DSYNC2;
        _myTZX.descriptor[currentBlock].timming.bit_0 = DBIT_0;
        _myTZX.descriptor[currentBlock].timming.bit_1 = DBIT_1;

        // Obtenemos el "pause despues del bloque"
        // BYTE 0x00 y 0x01
        _myTZX.descriptor[currentBlock].pauseAfterThisBlock = getDWORD(mFile,currentOffset+1);

        Serial.println("");
        Serial.println("Pause after block: " + String(_myTZX.descriptor[currentBlock].pauseAfterThisBlock));
        // Obtenemos el "tamaño de los datos"
        // BYTE 0x02 y 0x03
        _myTZX.descriptor[currentBlock].lengthOfData = getDWORD(mFile,currentOffset+3);

        Serial.println("");
        Serial.println("Length of data: ");
        Serial.print(_myTZX.descriptor[currentBlock].lengthOfData,HEX);

        // Los datos (TAP) empiezan en 0x04. Posición del bloque de datos.
        _myTZX.descriptor[currentBlock].offsetData = currentOffset + 5;

        Serial.println("");
        Serial.println("Offset of data: ");
        Serial.print(_myTZX.descriptor[currentBlock].offsetData,HEX);

        // Vamos a verificar el flagByte

        int flagByte = getBYTE(mFile,_myTZX.descriptor[currentBlock].offsetData);
        int typeBlock = getBYTE(mFile,_myTZX.descriptor[currentBlock].offsetData+1);

        if (flagByte < 128)
        {
            // Es una cabecera
            if (typeBlock == 0)
            {
                // Es una cabecera PROGRAM: BASIC
              Serial.println("");
              Serial.println("TYPEBLOCK: 0 - Header");
              Serial.println("");

                _myTZX.descriptor[currentBlock].header = true;
                _myTZX.descriptor[currentBlock].type = 0;

            }
            else if (typeBlock == 1)
            {

              Serial.println("");
              Serial.println("TYPEBLOCK: 1 - Header");
              Serial.println("");

                _myTZX.descriptor[currentBlock].header = true;
                _myTZX.descriptor[currentBlock].type = 0;
            }
            else if (typeBlock == 2)
            {
                Serial.println("");
                Serial.println("TYPEBLOCK: 2 - Header");
                Serial.println("");

                _myTZX.descriptor[currentBlock].header = true;
                _myTZX.descriptor[currentBlock].type = 0;
            }
            else if (typeBlock == 3)
            {

              Serial.println("");
              Serial.println("TYPEBLOCK: 3 - Header");
              Serial.println("");

              if (_myTZX.descriptor[currentBlock].lengthOfData == 6914)
              {
                  _myTZX.descriptor[currentBlock].screen = true;
                  _myTZX.descriptor[currentBlock].type = 7;
              }
              else
              {
                  // Es un bloque BYTE
                  _myTZX.descriptor[currentBlock].type = 1;                
              }
            }
        }
        else
        {


            if (typeBlock == 3)
            {
              Serial.println("");
              Serial.println("TYPEBLOCK: 3");
              Serial.println("");

              if (_myTZX.descriptor[currentBlock].lengthOfData == 6914)
              {
                  _myTZX.descriptor[currentBlock].screen = true;
                  _myTZX.descriptor[currentBlock].type = 3;
              }
              else
              {
                  // Es un bloque BYTE
                  _myTZX.descriptor[currentBlock].type = 4;                
              }
            }
            else
            {
                // Es un bloque BYTE
                _myTZX.descriptor[currentBlock].type = 4;
            }            
        } 

        // No contamos el ID
        // La cabecera tiene 4 bytes de parametros y N bytes de datos
        // pero para saber el total de bytes de datos hay que analizar el TAP
        // int positionOfTAPblock = currentOffset + 4;
        // dataTAPsize = getDWORD(mFile,positionOfTAPblock + headerTAPsize + 1);
        
        // NOTA: Sumamos 2 bytes que son la DWORD que indica el dataTAPsize
        _myTZX.descriptor[currentBlock].size = _myTZX.descriptor[currentBlock].lengthOfData; 
    }
    
    void analyzeID17(File32 mFile, int currentOffset, int currentBlock)
    {
        // Turbo data

        _myTZX.descriptor[currentBlock].ID = 17;
        _myTZX.descriptor[currentBlock].playeable = true;
        _myTZX.descriptor[currentBlock].offset = currentOffset;

        // Entonces
        // Timming de PULSE PILOT
        _myTZX.descriptor[currentBlock].timming.pulse_pilot = getDWORD(mFile,currentOffset+1);

        Serial.println("");
        Serial.print("PULSE PILOT=");
        Serial.print(_myTZX.descriptor[currentBlock].timming.pulse_pilot, HEX);

        // Timming de SYNC_1
        _myTZX.descriptor[currentBlock].timming.sync_1 = getDWORD(mFile,currentOffset+3);

          //Serial.println("");
          Serial.print(",SYNC1=");
          Serial.print(_myTZX.descriptor[currentBlock].timming.sync_1, HEX);

        // Timming de SYNC_2
        _myTZX.descriptor[currentBlock].timming.sync_2 = getDWORD(mFile,currentOffset+5);

          //Serial.println("");
          Serial.print(",SYNC2=");
          Serial.print(_myTZX.descriptor[currentBlock].timming.sync_2, HEX);

        // Timming de BIT 0
        _myTZX.descriptor[currentBlock].timming.bit_0 = getDWORD(mFile,currentOffset+7);

          //Serial.println("");
          Serial.print(",BIT_0=");
          Serial.print(_myTZX.descriptor[currentBlock].timming.bit_0, HEX);

        // Timming de BIT 1
        _myTZX.descriptor[currentBlock].timming.bit_1 = getDWORD(mFile,currentOffset+9);        

          //Serial.println("");
          Serial.print(",BIT_1=");
          Serial.print(_myTZX.descriptor[currentBlock].timming.bit_1, HEX);

        // Timming de PILOT TONE
        _myTZX.descriptor[currentBlock].timming.pilot_tone = getDWORD(mFile,currentOffset+11);

          //Serial.println("");
          Serial.print(",PILOT TONE=");
          Serial.print(_myTZX.descriptor[currentBlock].timming.pilot_tone, HEX);

        // Cogemos el byte de bits of the last byte
        _myTZX.descriptor[currentBlock].maskLastByte = getBYTE(mFile,currentOffset+13);
        zxp.set_maskLastByte(_myTZX.descriptor[currentBlock].maskLastByte);

          //Serial.println("");
          Serial.print(",MASK LAST BYTE=");
          Serial.print(_myTZX.descriptor[currentBlock].maskLastByte, HEX);


        // ********************************************************************
        //
        //
        //
        //
        //                 Ahora analizamos el BLOQUE DE DATOS
        //
        //
        //
        //
        // ********************************************************************

        // Obtenemos el "pause despues del bloque"
        // BYTE 0x00 y 0x01
        _myTZX.descriptor[currentBlock].pauseAfterThisBlock = getDWORD(mFile,currentOffset+14);

        Serial.println("");
        Serial.println("Pause after block: " + String(_myTZX.descriptor[currentBlock].pauseAfterThisBlock));
        // Obtenemos el "tamaño de los datos"
        // BYTE 0x02 y 0x03
        _myTZX.descriptor[currentBlock].lengthOfData = getNBYTE(mFile,currentOffset+16,3);

        Serial.println("");
        Serial.println("Length of data: ");
        Serial.print(String(_myTZX.descriptor[currentBlock].lengthOfData));

        // Los datos (TAP) empiezan en 0x04. Posición del bloque de datos.
        _myTZX.descriptor[currentBlock].offsetData = currentOffset + 19;

        Serial.println("");
        Serial.println("Offset of data: ");
        Serial.print(_myTZX.descriptor[currentBlock].offsetData,HEX);

        // Vamos a verificar el flagByte

        int flagByte = getBYTE(mFile,_myTZX.descriptor[currentBlock].offsetData);
        int typeBlock = getBYTE(mFile,_myTZX.descriptor[currentBlock].offsetData+1);

        if (flagByte < 128)
        {
            // Es una cabecera
            if (typeBlock == 0)
            {
                // Es una cabecera BASIC
                _myTZX.descriptor[currentBlock].header = true;
                _myTZX.descriptor[currentBlock].type = 0;

            }
            else if (typeBlock == 1)
            {
                _myTZX.descriptor[currentBlock].header = true;
                _myTZX.descriptor[currentBlock].type = 0;
            }
            else if (typeBlock == 2)
            {
                _myTZX.descriptor[currentBlock].header = true;
                _myTZX.descriptor[currentBlock].type = 0;
            }
            else if (typeBlock == 3)
            {
              if (_myTZX.descriptor[currentBlock].lengthOfData == 6914)
              {
                  _myTZX.descriptor[currentBlock].screen = true;
                  _myTZX.descriptor[currentBlock].type = 7;
              }
              else
              {
                  // Es un bloque BYTE
                  _myTZX.descriptor[currentBlock].type = 1;                
              }
            }
        }
        else
        {
            // Es un bloque BYTE
            _myTZX.descriptor[currentBlock].type = 4;
        }

        // No contamos el ID
        // La cabecera tiene 4 bytes de parametros y N bytes de datos
        // pero para saber el total de bytes de datos hay que analizar el TAP
        // int positionOfTAPblock = currentOffset + 4;
        // dataTAPsize = getDWORD(mFile,positionOfTAPblock + headerTAPsize + 1);
        
        // NOTA: Sumamos 2 bytes que son la DWORD que indica el dataTAPsize
        _myTZX.descriptor[currentBlock].size = _myTZX.descriptor[currentBlock].lengthOfData; 


    }

    void analyzeID18(File32 mFile, int currentOffset, int currentBlock)
    {
        // pure tone

        _myTZX.descriptor[currentBlock].ID = 18;
        _myTZX.descriptor[currentBlock].playeable = false;
        _myTZX.descriptor[currentBlock].forSetting = true;
        _myTZX.descriptor[currentBlock].offset = currentOffset;    

        // Timming de PILOT TONE
        _myTZX.descriptor[currentBlock].timming.pilot_tone = getDWORD(mFile,currentOffset+1) * getDWORD(mFile,currentOffset+3);          
    }

    void analyzeID32(File32 mFile, int currentOffset, int currentBlock)
    {
        // Con este ID analizamos la parte solo del timming ya que el resto
        // información del bloque se puede analizar con "analyzeID16"

        _myTZX.descriptor[currentBlock].ID = 32;
        _myTZX.descriptor[currentBlock].playeable = false;
        _myTZX.descriptor[currentBlock].forSetting = true;
        _myTZX.descriptor[currentBlock].offset = currentOffset;    

        // Timming de PILOT TONE
        _myTZX.descriptor[currentBlock].pauseAfterThisBlock = getDWORD(mFile,currentOffset+1);          
    }

    void analyzeID50(File32 mFile, int currentOffset, int currentBlock)
    {
        // Information block
        int sizeBlock = 0;

        _myTZX.descriptor[currentBlock].ID = 50;
        _myTZX.descriptor[currentBlock].playeable = false;
        // Los dos primeros bytes del bloque no se cuentan para
        // el tamaño total
        _myTZX.descriptor[currentBlock].offset = currentOffset+3;

        sizeBlock = getDWORD(mFile,currentOffset+1);

        //Serial.println("");
        //Serial.println("ID48 - TextSize: " + String(sizeTextInformation));
        
        // El tamaño del bloque es "1 byte de longitud de texto + TAMAÑO_TEXTO"
        // el bloque comienza en el offset del ID y acaba en
        // offset[ID] + tamaño_bloque
        _myTZX.descriptor[currentBlock].size = sizeBlock;         
    }    

    void analyzeID48(File32 mFile, int currentOffset, int currentBlock)
    {
        // Information block
        int sizeTextInformation = 0;

        _myTZX.descriptor[currentBlock].ID = 48;
        _myTZX.descriptor[currentBlock].playeable = false;
        _myTZX.descriptor[currentBlock].offset = currentOffset;

        sizeTextInformation = getBYTE(mFile,currentOffset+1);

        Serial.println("");
        Serial.println("ID48 - TextSize: " + String(sizeTextInformation));
        
        // El tamaño del bloque es "1 byte de longitud de texto + TAMAÑO_TEXTO"
        // el bloque comienza en el offset del ID y acaba en
        // offset[ID] + tamaño_bloque
        _myTZX.descriptor[currentBlock].size = sizeTextInformation + 1;
    }

    void analyzeID33(File32 mFile, int currentOffset, int currentBlock)
    {
        // Group start
        int sizeTextInformation = 0;

        _myTZX.descriptor[currentBlock].ID = 33;
        _myTZX.descriptor[currentBlock].playeable = false;
        _myTZX.descriptor[currentBlock].offset = currentOffset+2;

        sizeTextInformation = getBYTE(mFile,currentOffset+1);

        Serial.println("");
        Serial.println("ID33 - TextSize: " + String(sizeTextInformation));
        
        // El tamaño del bloque es "1 byte de longitud de texto + TAMAÑO_TEXTO"
        // el bloque comienza en el offset del ID y acaba en
        // offset[ID] + tamaño_bloque
        _myTZX.descriptor[currentBlock].size = sizeTextInformation;
    }

    void analyzeBlockUnknow(int id_num, int currentOffset, int currentBlock)
    {
        _myTZX.descriptor[currentBlock].ID = id_num;
        _myTZX.descriptor[currentBlock].playeable = false;
        _myTZX.descriptor[currentBlock].offset = currentOffset;      
    }

    void getBlockDescriptor(File32 mFile, int sizeTZX)
    {
          // Para ello tenemos que ir leyendo el TZX poco a poco
          // Detectaremos los IDs a partir del byte 9 (empezando por offset = 0)
          // Cada bloque empieza por un ID menos el primero 
          // que empieza por ZXTape!
          // Ejemplo ID + bytes
          //
          int startOffset = 10;   // Posición del primero ID, pero el offset
                                  // 0x00 de cada ID, sería el siguiente
          int currentID = 0;
          int nextIDoffset = 0;
          int currentOffset = 0;
          int lenghtDataBlock = 0;
          int currentBlock = 1;
          bool endTZX = false;
          bool endWithErrors = false;

          currentOffset = startOffset;

          // Comenzamos a rastrear el fichero
          
          if (_myTZX.descriptor != NULL)
          {
              free(_myTZX.descriptor);
              _myTZX.descriptor = NULL;
          }

          // Reservamos espacio para el primer bloque y después iremos extendiendo
          // Vamos a ir usando realloc() para ir incrementando el espacio en memoria de manera dinámica.

          _myTZX.descriptor = (tTZXBlockDescriptor*)calloc(3000,sizeof(tTZXBlockDescriptor));              

          TIMMING_STABLISHED = false;

          while (!endTZX)
          {
              // El objetivo es ENCONTRAR IDs y ultimo byte, y analizar el bloque completo para el descriptor.
              currentID = getID(mFile, currentOffset);
              
              Serial.println("");
              Serial.println("-----------------------------------------------");
              Serial.println("TZX ID ");
              Serial.print(currentID, HEX);
              Serial.println("");
              Serial.println("block: " + String(currentBlock));
              Serial.println("");

              // Ahora dependiendo del ID analizamos. Los ID están en HEX
              // y la rutina devolverá la posición del siguiente ID, así hasta
              // completar todo el fichero
              switch (currentID)
              {
                // ID 10 - Standard Speed Data Block
                case 16:

                  if (_myTZX.descriptor != NULL)
                  {
                      // Obtenemos la dirección del siguiente offset
                      analyzeID16(mFile,currentOffset, currentBlock);

                      nextIDoffset = currentOffset + _myTZX.descriptor[currentBlock].size + 5;
                      
                      currentBlock++;
                  }
                  else
                  {
                      Serial.println("");
                      Serial.println("Error: Not allocation memory for block ID 0x10");
                      endTZX = true;
                      endWithErrors = true;
                  }
                break;

                // ID 11- Turbo Speed Data Block
                case 17:
                  
                  TIMMING_STABLISHED = true;
                  
                  if (_myTZX.descriptor != NULL)
                  {
                      // Obtenemos la dirección del siguiente offset
                      analyzeID17(mFile,currentOffset, currentBlock);

                      nextIDoffset = currentOffset + _myTZX.descriptor[currentBlock].size + 19;
                      
                      currentBlock++;
                  }
                  else
                  {
                      Serial.println("");
                      Serial.println("Error: Not allocation memory for block ID 0x11");
                      endTZX = true;
                      endWithErrors = true;
                  }
                break;

                // ID 12 - Pure Tone
                case 18:
                  //analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  if (_myTZX.descriptor != NULL)
                  {
                      // Obtenemos la dirección del siguiente offset
                      analyzeID16(mFile,currentOffset, currentBlock);

                      nextIDoffset = currentOffset + _myTZX.descriptor[currentBlock].size + 4;
                      
                      currentBlock++;
                  }
                  else
                  {
                      Serial.println("");
                      Serial.println("Error: Not allocation memory for block ID 0x12");
                      endTZX = true;
                      endWithErrors = true;
                  }
                break;

                // ID 13 - Pulse sequence
                case 19:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 14 - Pure Data Block
                case 20:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 15 - Direct Recording
                case 21:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 18 - CSW Recording
                case 24:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 19 - Generalized Data Block
                case 25:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 20 - Pause and Stop Tape
                case 32:
                  //analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  if (_myTZX.descriptor != NULL)
                  {
                      // Obtenemos la dirección del siguiente offset
                      analyzeID32(mFile,currentOffset, currentBlock);

                      nextIDoffset = currentOffset + _myTZX.descriptor[currentBlock].size + 3;
                      
                      currentBlock++;
                  }
                  else
                  {
                      Serial.println("");
                      Serial.println("Error: Not allocation memory for block ID 0x20");
                      endTZX = true;
                      endWithErrors = true;
                  }
                  break;

                // ID 21 - Group start
                case 33:
                  if (_myTZX.descriptor != NULL)
                  {
                      // Obtenemos la dirección del siguiente offset
                      analyzeID33(mFile,currentOffset, currentBlock);

                      nextIDoffset = currentOffset + 2 + _myTZX.descriptor[currentBlock].size;
                      
                      currentBlock++;
                  }
                  else
                  {
                      Serial.println("");
                      Serial.println("Error: Not allocation memory for block ID 0x21");
                      endTZX = true;
                      endWithErrors = true;
                  }
                  break;                

                // ID 22 - Group end
                case 34:
                  if (_myTZX.descriptor != NULL)
                  {
                      // Obtenemos la dirección del siguiente offset
                      _myTZX.descriptor[currentBlock].ID = 34;
                      _myTZX.descriptor[currentBlock].playeable = false;
                      _myTZX.descriptor[currentBlock].offset = currentOffset;

                      nextIDoffset = currentOffset + 1;                      
                      currentBlock++;
                  }
                  else
                  {
                      Serial.println("");
                      Serial.println("Error: Not allocation memory for block ID 0x22");
                      endTZX = true;
                      endWithErrors = true;
                  }
                  break;

                // ID 23 - Jump to block
                case 35:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 24 - Loop start
                case 36:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 25 - Loop end
                case 37:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 26 - Call sequence
                case 38:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 27 - Return from sequence
                case 39:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 28 - Select block
                case 40:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 2A - Stop the tape if in 48K mode
                case 42:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 2B - Set signal level
                case 43:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 30 - Information
                case 48:
                  // No hacemos nada solamente coger el siguiente offset
                  if (_myTZX.descriptor != NULL)
                  {
                      // Obtenemos la dirección del siguiente offset
                      analyzeID48(mFile,currentOffset,currentBlock);

                      // Siguiente ID
                      nextIDoffset = currentOffset + _myTZX.descriptor[currentBlock].size + 1;

                      currentBlock++;
                  }
                  else
                  {
                      Serial.println("");
                      Serial.println("Error: Not allocation memory for block ID 0x10");
                      endTZX = true;
                      endWithErrors = true;
                  }                  
                  break;

                // ID 31 - Message block
                case 49:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 32 - Archive info
                case 50:
                  // No hacemos nada solamente coger el siguiente offset
                  if (_myTZX.descriptor != NULL)
                  {
                      // Obtenemos la dirección del siguiente offset
                      analyzeID50(mFile,currentOffset,currentBlock);

                      // Siguiente ID
                      nextIDoffset = currentOffset + 3 + _myTZX.descriptor[currentBlock].size;

                      currentBlock++;
                  }
                  else
                  {
                      Serial.println("");
                      Serial.println("Error: Not allocation memory for block ID 0x10");
                      endTZX = true;
                      endWithErrors = true;
                  }                  
                  break;

                // ID 33 - Hardware type
                case 51:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 35 - Custom info block
                case 53:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                // ID 5A - "Glue" block (90 dec, ASCII Letter 'Z')
                case 90:
                  analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  currentBlock++;
                  break;

                default:
                  Serial.println("");
                  Serial.println("ID unknow " + currentID);
                  //analyzeBlockUnknow(currentID,currentOffset, currentBlock);
                  break;
          }

              Serial.println("");
              Serial.println("Next ID offset: ");
              Serial.print(nextIDoffset, HEX);
              
              //_myTZX.descriptor = (tTZXBlockDescriptor*)realloc(_myTZX.descriptor,currentBlock + 1); 

              if (nextIDoffset >= sizeTZX)
              {
                  // Finalizamos
                  endTZX = true;
              }
              else
              {
                  currentOffset = nextIDoffset;
              }

              //delay(2000);
              TOTAL_BLOCKS = currentBlock;
              _hmi.updateInformationMainPage();

              Serial.println("");
              Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++");
              Serial.println("");

          }
          // El siguiente bloque será
          // currentOffset + blockSize (que obtenemos del descriptor)
          Serial.println("");
          Serial.println("End: Total blocks " + String(currentBlock));

          _myTZX.numBlocks = currentBlock;
          _myTZX.size = sizeTZX;
          _myTZX.name = "TZX";
          
          // Actualizamos una vez mas el HMI
          if (currentBlock % 100 == 0)
          {
              _hmi.updateInformationMainPage();
          }
    }
   
    void proccess_tzx(File32 tzxFileName)
    {
          // Procesamos el fichero
          Serial.println("");
          Serial.println("Getting total blocks...");     

        if (isFileTZX(tzxFileName))
        {
            Serial.println();
            Serial.println("Is TZX file");

            Serial.println();
            Serial.println("Size: " + String(_sizeTZX));

            getBlockDescriptor(_mFile,_sizeTZX);
        }      
    }

    void sendStatus(int action, int value) 
    {

        switch (action) {
          case PLAY_ST:
            //_hmi.writeString("");
            _hmi.writeString("PLAYst.val=" + String(value));
            break;

          case STOP_ST:
            //_hmi.writeString("");
            _hmi.writeString("STOPst.val=" + String(value));
            break;

          case PAUSE_ST:
            //_hmi.writeString("");
            _hmi.writeString("PAUSEst.val=" + String(value));
            break;

          case END_ST:
            //_hmi.writeString("");
            _hmi.writeString("ENDst.val=" + String(value));
            break;

          case READY_ST:
            //_hmi.writeString("");
            _hmi.writeString("READYst.val=" + String(value));
            break;

          case ACK_LCD:
            //_hmi.writeString("");
            _hmi.writeString("tape.LCDACK.val=" + String(value));
            //_hmi.writeString("");
            _hmi.writeString("statusLCD.txt=\"READY. PRESS SCREEN\"");
            break;
          
          case RESET:
            //_hmi.writeString("");
            _hmi.writeString("statusLCD.txt=\"SYSTEM REBOOT\"");
        }
    }

    void showBufferPlay(byte* buffer, int size)
    {
        Serial.println("Listing bufferplay.");
        for (int n=0;n<size;n++)
        {
            Serial.print(buffer[n],HEX);
            Serial.print(",");
        }
        Serial.println("");
    }

    public:

    void showInfoBlockInProgress(int nBlock)
    {
        switch(nBlock)
        {
            case 0:

                // Definimos el buffer del PLAYER igual al tamaño del bloque
                #if LOG==3
                  Serial.println("> PROGRAM HEADER");
                #endif
                LAST_TYPE = &LASTYPE0[0];
                break;

            case 1:

                // Definimos el buffer del PLAYER igual al tamaño del bloque
                #if LOG==3
                  Serial.println("");
                  Serial.println("> BYTE HEADER");
                #endif
                LAST_TYPE = &LASTYPE1[0];
                break;

            case 7:

                // Definimos el buffer del PLAYER igual al tamaño del bloque
                #if LOG==3
                  Serial.println("");
                  Serial.println("> SCREEN HEADER");
                #endif
                LAST_TYPE = &LASTYPE7[0];
                break;

            case 2:
                // Definimos el buffer del PLAYER igual al tamaño del bloque
                #if LOG==3
                  Serial.println("");
                  Serial.println("> BASIC PROGRAM");
                #endif
                LAST_TYPE = &LASTYPE2[0];
                break;

            case 3:
                // Definimos el buffer del PLAYER igual al tamaño del bloque
                #if LOG==3
                  Serial.println("");
                  Serial.println("> SCREEN");
                #endif
                LAST_TYPE = &LASTYPE3[0];
                break;

            case 4:
                // Definimos el buffer del PLAYER igual al tamaño del bloque
                #if LOG==3
                  Serial.println("");
                  Serial.println("> BYTE CODE");
                #endif
                if (LAST_SIZE != 6914)
                {
                    LAST_TYPE = &LASTYPE4_1[0];
                }
                else
                {
                    LAST_TYPE = &LASTYPE4_2[0];
                }
                break;

            case 5:
                // Definimos el buffer del PLAYER igual al tamaño del bloque
                #if LOG==3
                  Serial.println("");
                  Serial.println("> ARRAY.NUM");
                #endif
                LAST_TYPE = &LASTYPE5[0];
                break;

            case 6:
                // Definimos el buffer del PLAYER igual al tamaño del bloque
                #if LOG==3
                  Serial.println("");
                  Serial.println("> ARRAY.CHR");
                #endif
                LAST_TYPE = &LASTYPE6[0];
                break;


        }        
    }

    void set_SDM(SDmanager sdmTmp)
    {
        //_sdm = sdmTmp;
        //ptrSdm = &sdmTmp;
    }

    void set_SdFat32(SdFat32 sdf32)
    {
        _sdf32 = sdf32;
    }

    void set_HMI(HMI hmi)
    {
        _hmi = hmi;
    }

    void set_file(File32 mFile, int sizeTZX)
    {
      // Pasamos los parametros a la clase
      _mFile = mFile;
      _sizeTZX = sizeTZX;
    }  

    void getInfoFileTZX(char* path) 
    {
      
      File32 tzxFile;

      LAST_MESSAGE = "Analyzing file";
      _hmi.updateInformationMainPage();
    
      // Abrimos el fichero
      tzxFile = sdm.openFile32(tzxFile, path);

      // Obtenemos su tamaño total
      _mFile = tzxFile;
      _rlen = tzxFile.available();

      // creamos un objeto TZXproccesor
      set_file(tzxFile, _rlen);
      proccess_tzx(tzxFile);
      
      Serial.println("");
      Serial.println("END PROCCESING TZX: ");
    
    }

    void initialize()
    {
        if (_myTZX.descriptor != NULL)
        {
          free(_myTZX.descriptor);
          free(_myTZX.name);
          _myTZX.descriptor = NULL;
          _myTZX.name = "\0";
          _myTZX.numBlocks = 0;
          _myTZX.size = 0;
        }          
    }

    void play()
    {
        //_hmi.writeString("");
        _hmi.writeString("READYst.val=0");

        //_hmi.writeString("");
        _hmi.writeString("ENDst.val=0");

        if (_myTZX.descriptor != NULL)
        {         
              // Inicializamos el buffer de reproducción. Memoria dinamica
              byte* bufferPlay = NULL;

              // Entregamos información por consola
              PROGRAM_NAME = _myTZX.name;
              TOTAL_BLOCKS = _myTZX.numBlocks;
              LAST_NAME = &INITCHAR2[0];

              // Ahora reproducimos todos los bloques desde el seleccionado (para cuando se quiera uno concreto)
              int m = BLOCK_SELECTED;
              //BYTES_TOBE_LOAD = _rlen;

              // Reiniciamos
              if (BLOCK_SELECTED == 0) {
                BYTES_LOADED = 0;
                BYTES_TOBE_LOAD = _rlen;
                //_hmi.writeString("");
                _hmi.writeString("progressTotal.val=" + String((int)((BYTES_LOADED * 100) / (BYTES_TOBE_LOAD))));
              } else {
                BYTES_TOBE_LOAD = _rlen - _myTZX.descriptor[BLOCK_SELECTED - 1].offset;
              }

              for (int i = m; i < _myTZX.numBlocks; i++) 
              {
                
                Serial.println("");
                Serial.println("");
                Serial.println("");
                Serial.println("");
                Serial.println("++++++++++++++++++++++++++++++++++++++++++++");
                Serial.println("> BLOCK " + String(i));
                Serial.print("> ID ");
                Serial.print(_myTZX.descriptor[i].ID,HEX);
                Serial.print("> Offset: ");
                Serial.print(_myTZX.descriptor[i].offset,HEX);
                Serial.print("> Pause after block: ");
                Serial.print(_myTZX.descriptor[i].pauseAfterThisBlock,HEX);
                Serial.println("");
                Serial.println("");

                if (_myTZX.descriptor[i].ID == 18)
                {
                    // Reproducimos un tono puro
                    zxp.pilotTone(_myTZX.descriptor[i].timming.pilot_tone);
                }

                if (_myTZX.descriptor[i].ID == 32)
                {
                    // Hacemos un delay
                    int dly = _myTZX.descriptor[i].pauseAfterThisBlock;
                    if (dly == 0)
                    {
                        // En el caso de cero. Paramos el TAPE
                        i = _myTZX.numBlocks;
                        break;
                    }
                    else
                    {
                        // En otro caso. Delay
                        zxp.silence(dly);
                    }                    
                }


                if (_myTZX.descriptor[i].playeable)
                {
                      //Silent
                      zxp.silent = _myTZX.descriptor[i].pauseAfterThisBlock;        



                      //SYNC1
                      zxp.SYNC1 = _myTZX.descriptor[i].timming.sync_1;
                      //SYNC2
                      zxp.SYNC2 = _myTZX.descriptor[i].timming.sync_2;
                      //PULSE PILOT
                      zxp.PULSE_PILOT = _myTZX.descriptor[i].timming.pulse_pilot;
                      // BTI 0
                      zxp.BIT_0 = _myTZX.descriptor[i].timming.bit_0;
                      // BIT1                                          
                      zxp.BIT_1 = _myTZX.descriptor[i].timming.bit_1;

                      //LAST_NAME = bDscr[i].name;
                      // Obtenemos el nombre del bloque
                      LAST_NAME = _myTZX.descriptor[i].name;
                      LAST_SIZE = _myTZX.descriptor[i].size;

                      Serial.println("");
                      Serial.println("");
                      Serial.println("Playing TZX");
                      Serial.println("+ Name: " + String(LAST_NAME));
                      Serial.println("+ Size: " + String(LAST_SIZE));

                      // Almacenmas el bloque en curso para un posible PAUSE
                      if (LOADING_STATE != 2) {
                        CURRENT_BLOCK_IN_PROGRESS = i;
                        BLOCK_SELECTED = i;

                        //_hmi.writeString("");
                        _hmi.writeString("currentBlock.val=" + String(i + 1));

                        //_hmi.writeString("");
                        _hmi.writeString("progression.val=" + String(0));
                      }

                      //Paramos la reproducción.
                      if (LOADING_STATE == 2) {
                        PAUSE = false;
                        STOP = false;
                        PLAY = false;
                        LOADING_STATE = 0;
                        break;
                      }

                      //Ahora vamos lanzando bloques dependiendo de su tipo
                      //Esto actualiza el LAST_TYPE
                      showInfoBlockInProgress(_myTZX.descriptor[i].type);

                      // Actualizamos HMI
                      _hmi.setBasicFileInformation(_myTZX.descriptor[BLOCK_SELECTED].name,_myTZX.descriptor[BLOCK_SELECTED].typeName,_myTZX.descriptor[BLOCK_SELECTED].size);

                      _hmi.updateInformationMainPage();

                      // Reproducimos el fichero
                      if (_myTZX.descriptor[i].type == 0) 
                      {
                        
                        // CABECERAS
                        if(bufferPlay!=NULL)
                        {
                            free(bufferPlay);
                            bufferPlay=NULL;

                        }

                        Serial.println("");
                        Serial.println("Head 1" + _myTZX.descriptor[i].ID);
                        Serial.println("");

                        bufferPlay = (byte*)calloc(_myTZX.descriptor[i].size, sizeof(byte));

                        bufferPlay = sdm.readFileRange32(_mFile, _myTZX.descriptor[i].offsetData, _myTZX.descriptor[i].size, false);

                        showBufferPlay(bufferPlay,_myTZX.descriptor[i].size);
                        verifyChecksum(_mFile,_myTZX.descriptor[i].offsetData,_myTZX.descriptor[i].size);


                        switch (_myTZX.descriptor[i].ID)
                        {
                          case 16:
                            //PULSE PILOT
                            _myTZX.descriptor[i].timming.pilot_tone = DPILOT_HEADER;
                            zxp.PILOT_TONE = _myTZX.descriptor[i].timming.pilot_tone;
                            break;

                          case 17:
                            //PULSE PILOT
                            zxp.PILOT_TONE = _myTZX.descriptor[i].timming.pilot_tone;
                            break;
                        }

                        // Llamamos a la clase de reproducción
                        // Cabecera PROGRAM
                        zxp.playData(bufferPlay, _myTZX.descriptor[i].size,_myTZX.descriptor[i].timming.pilot_tone * _myTZX.descriptor[i].timming.pulse_pilot);

                      } 
                      else if (_myTZX.descriptor[i].type == 1 || _myTZX.descriptor[i].type == 7) 
                      {
                        
                        // CABECERAS
                        if(bufferPlay!=NULL)
                        {
                            free(bufferPlay);
                            bufferPlay=NULL;
                        }      

                        Serial.println("");
                        Serial.println("Head 2" + _myTZX.descriptor[i].ID);
                        Serial.println("");

                        bufferPlay = (byte*)calloc(_myTZX.descriptor[i].size, sizeof(byte));

                        bufferPlay = sdm.readFileRange32(_mFile, _myTZX.descriptor[i].offsetData, _myTZX.descriptor[i].size, false);

                        showBufferPlay(bufferPlay,_myTZX.descriptor[i].size);
                        verifyChecksum(_mFile,_myTZX.descriptor[i].offsetData,_myTZX.descriptor[i].size);


                        // Llamamos a la clase de reproducción
                        switch (_myTZX.descriptor[i].ID)
                        {
                          case 16:
                            //PULSE PILOT
                            _myTZX.descriptor[i].timming.pilot_tone = DPILOT_HEADER;
                            zxp.PILOT_TONE = _myTZX.descriptor[i].timming.pilot_tone;
                            break;

                          case 17:
                            //PULSE PILOT
                            zxp.PILOT_TONE = _myTZX.descriptor[i].timming.pilot_tone;
                            break;
                        }
                        
                        // Cabecera BYTE
                        zxp.playData(bufferPlay, _myTZX.descriptor[i].size,_myTZX.descriptor[i].timming.pilot_tone * _myTZX.descriptor[i].timming.pulse_pilot);
                      } 
                      else 
                      {
                        // DATA
                        int blockSize = _myTZX.descriptor[i].size;

                        if(bufferPlay!=NULL)
                        {
                            free(bufferPlay);
                            bufferPlay=NULL;
                        }

                        bufferPlay = (byte*)calloc(_myTZX.descriptor[i].size, sizeof(byte));

                        bufferPlay = sdm.readFileRange32(_mFile, _myTZX.descriptor[i].offsetData, _myTZX.descriptor[i].size, true);


                        Serial.println("");
                        Serial.println("Data ");
                        Serial.println("");

                        showBufferPlay(bufferPlay,_myTZX.descriptor[i].size);
                        verifyChecksum(_mFile,_myTZX.descriptor[i].offsetData,_myTZX.descriptor[i].size);


                        Serial.println("");
                        Serial.println("ID:");
                        Serial.println(_myTZX.descriptor[i].ID,HEX);
                        Serial.println("");

                        switch (_myTZX.descriptor[i].ID)
                        {
                          case 16:
                            //PULSE PILOT
                            _myTZX.descriptor[i].timming.pilot_tone = DPILOT_DATA;
                            zxp.PILOT_TONE = _myTZX.descriptor[i].timming.pilot_tone;
                            break;

                          case 17:
                            //PULSE PILOT
                            zxp.PILOT_TONE = _myTZX.descriptor[i].timming.pilot_tone;
                            break;
                        }
                                               
                        Serial.println("");
                        Serial.println("PILOT TONE:");
                        Serial.println(String(_myTZX.descriptor[i].timming.pilot_tone));


                        Serial.println("");
                        Serial.println("PULSE PILOT:");
                        Serial.println(String(_myTZX.descriptor[i].timming.pulse_pilot));

                        Serial.println("");

                        // Bloque de datos BYTE
                        zxp.playData(bufferPlay, _myTZX.descriptor[i].size,_myTZX.descriptor[i].timming.pilot_tone * _myTZX.descriptor[i].timming.pulse_pilot);
                      }                  
                }

                Serial.println("");
                Serial.println("--------------------------------------------");
                Serial.println("");
                Serial.println("");
                Serial.println("");
                Serial.println("");
                Serial.println("");
                
              }

              Serial.println("");
              Serial.println("Playing was finish.");
              // En el caso de no haber parado manualmente, es por finalizar
              // la reproducción
              if (LOADING_STATE == 1) {
                PLAY = false;
                STOP = true;
                PAUSE = false;
                BLOCK_SELECTED = 0;
                LAST_MESSAGE = "Playing end. Automatic STOP.";

                _hmi.setBasicFileInformation(_myTZX.descriptor[BLOCK_SELECTED].name,_myTZX.descriptor[BLOCK_SELECTED].typeName,_myTZX.descriptor[BLOCK_SELECTED].size);

                _hmi.updateInformationMainPage();

                // Ahora ya podemos tocar el HMI panel otra vez
                sendStatus(END_ST, 1);
              }

              // Cerrando
              LOADING_STATE = 0;

              if(bufferPlay!=NULL)
              {
                free(bufferPlay);
                bufferPlay=NULL;
              }
              
              Serial.flush();

              // Ahora ya podemos tocar el HMI panel otra vez
              sendStatus(READY_ST, 1);

        }
        else
        {
            LAST_MESSAGE = "No file selected.";
            _hmi.setBasicFileInformation(_myTZX.descriptor[BLOCK_SELECTED].name,_myTZX.descriptor[BLOCK_SELECTED].typeName,_myTZX.descriptor[BLOCK_SELECTED].size);

            _hmi.updateInformationMainPage();
        }        

    }

    TZXproccesor(AudioKit kit)
    {
        // Constructor de la clase
        _myTZX.name = (char*)calloc(10+1,sizeof(char));
        _myTZX.name = &INITCHAR[0];
        _myTZX.numBlocks = 0;
        _myTZX.descriptor = NULL;
        _myTZX.size = 0;

        //zxp.set_ESP32kit(kit);      
    } 


};