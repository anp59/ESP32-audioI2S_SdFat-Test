//**********************************************************************************************************
//*    audioI2S-- I2S audiodecoder for ESP32, *
//**********************************************************************************************************
//
// first release on 11/2018
// Version 3  , Jul.02/2020
//
//
// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR
// COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE
//

#include "Arduino.h"
#include "Audio.h"
#include "WiFiMulti.h"

#if USE_SDFAT  // set in platformio.ini
#include "Sdfat.h"
fs::SDFATFS SD;
#else
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#endif

// Digital I/O used
#ifdef CONFIG_IDF_TARGET_ESP32S3
// !! GPIO 33-37 not available for OPI PSRAM !!
#define SD_CS 10
#define SPI_SCK 12
#define SPI_MISO 13
#define SPI_MOSI 11
#define I2S_DOUT 5
#define I2S_BCLK 6
#define I2S_LRC 4
#else
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26
#endif

Audio audio;

const char *playList[] = {
    "Test2/06 - Rockerrente_k.mp3",
    "Test2/25. Alvaro Soler - Sofia_k.m4a",
    "Musik/Max Raabe/max raabe - das fräulein gerda.mp3",
    "Musik/André Rieu/Bal Du Siècle/01 Clavelitos.mp3",
    "Musik/André Rieu/Bal Du Siècle/02 España Cañi.mp3",
    "Musik/André Rieu/Bal Du Siècle/03 Cielito Lindo.mp3",
    // "Musik/André Rieu/Bal Du Siècle/08 Moulin Rouge; Paris Canaille.mp3",
    // "Musik/André Rieu/Bal Du Siècle/06 Hava Nagila.mp3",
    // "Musik/André Rieu/Bal Du Siècle/17 La Vie En Rose; Padam Padam; Sous Les
    // Ponts De Paris.mp3", "Musik1/Deutschland/Ostrock/Silly - Best of Silly/01
    // - Bye Bye.mp3", "Musik1/Deutschland/Ostrock/Silly - Best of Silly/11 -
    // Instandbesetzt.mp3", "Musik1/Deutschland/Ostrock/Puhdys - SUPERillu/03 -
    // He, John.mp3", "Musik1/Deutschland/Ostrock/Puhdys - SUPERillu/06 -
    "Test2/01 - Bye Bye_k.mp3", 
    "Test2/03 - He, John_k.mp3"
}; 

bool f_eof = false;

void playNext(int offset = 0) {
    static int i = 0;
    size_t lsz;

    if (lsz = sizeof(playList) / sizeof(char *)) {
        log_i("playList_size -> %d", lsz);
        i += offset;
        i = (lsz + i) % lsz;
        if (audio.connecttoFS(SD, playList[i]))
            Serial.printf("\n** audio.connecttoFS - playList[%d] = %s\n", i,
                          playList[i]);
        else
            log_e("audio.connecttoFS not successful- playList[%d] = %s\n", i,
                  playList[i]);
    }
    else
        log_w("playList is empty");
}

void setup() {
    Serial.begin(115200);
    while ( !Serial );
#if !USE_SDFAT
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);
#endif
    SD.begin(SD_CS);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(12);  // 0...21
    log_i("Using %s...", USE_SDFAT ? "SdFat" : "FS") ;
    playNext();

#if USE_SDFAT
    // use SdFat files
    File f1;
    File32 f2;
    f1.open(playList[4]);
    Serial.println(f1.name());
    f1.close();
    f2.open(playList[5]);
    Serial.println(f2.name());
    f2.close();
#endif
}

void loop() {
    audio.loop();
    if (f_eof) {
        f_eof = false;
        audio.stopSong();
        playNext(1);
        log_i("free heap=%i", ESP.getFreeHeap());
    }
    else {
        if (Serial.available()) {
            audio.stopSong();
            int i = 0;
            String r = Serial.readString();
            r.trim();
            i = r.toInt();
            playNext(i);
            log_i("free heap=%i", ESP.getFreeHeap());
        }
    }
}

// optional
void audio_info(const char *info) {
    Serial.print("info        ");
    Serial.println(info);
}
void audio_id3data(const char *info) {  // id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}
void audio_eof_mp3(const char *info) {  // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
    f_eof = true;
}
void audio_showstation(const char *info) {
    Serial.print("station     ");
    Serial.println(info);
}
void audio_showstreamtitle(const char *info) {
    Serial.print("streamtitle ");
    Serial.println(info);
}
void audio_bitrate(const char *info) {
    Serial.print("bitrate     ");
    Serial.println(info);
}
void audio_commercial(const char *info) {  // duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}
void audio_icyurl(const char *info) {  // homepage
    Serial.print("icyurl      ");
    Serial.println(info);
}
void audio_lasthost(const char *info) {  // stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}
