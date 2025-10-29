#pragma once
#include <string>
#include <algorithm>

struct RhythmData {
    static const int NUM_STYLES = 6;
    static const int NUM_DRUMS = 6;
    static const int NUM_PATTERNS = 8;
    static const int MAX_STEPS = 16;

    // rhythms[style][drum][pattern][step]

    //Styles order: Bulgar/Fraylach, Araber, Terkisher, Zhok/Hora, In Zibn
    bool rhythms[NUM_STYLES][NUM_DRUMS][NUM_PATTERNS][MAX_STEPS] = {};
    int sequenceLengths[NUM_STYLES] = {16, 16, 16, 16, 12, 14}; // Sequence step length 

    RhythmData() {
        initRhythms();
    }

    // Load pattern from a string of '0' and '1'
    void loadRhythmFromString(int style, int drum, int pattern, const std::string& patternStr) {
        int len = std::min((int)patternStr.size(), MAX_STEPS);
        for (int i = 0; i < len; i++)
            rhythms[style][drum][pattern][i] = (patternStr[i] == '1');
    }

	/*
	========================
	Tantz Drum Sequencer Mapping
	========================

	--- Styles (STYLE_PARAM) ---
	0 = Bulgar
	1 = Bulgar #2
    2 = Araber
    3 = Terkisher
    4 = Hora
    5 = In Zibn

	--- Drums ---
	0 = Kick
	1 = Snare
	2 = Hi-Hat Closed
	3 = Hi-Hat Open
	4 = Percussion 1
	5 = Percussion 2

	--- Patterns per Drum ---
	0 = Pattern 1
	1 = Pattern 2
	2 = Pattern 3
	3 = Pattern 4
	4 = Pattern 5
	5 = Pattern 6
	6 = Pattern 7
	7 = Pattern 8

	--- Pattern String ---
	"1" = gate ON
	"0" = gate OFF
	Length = up to MAX_STEPS (16 by default)
	Example: "1000100010001000" → hits on steps 1, 5, 9, 13

	========================
	Usage:
	loadRhythmFromString(style, drum, pattern, "1010001010001000");
	========================
    */

    void initRhythms() {

        // Stle 0: Bulgar #1 
        // Kick
        loadRhythmFromString(0, 0, 0, "1000000000001000");
        loadRhythmFromString(0, 0, 1, "1000001000001000");
        loadRhythmFromString(0, 0, 2, "1000100010001000");
        loadRhythmFromString(0, 0, 3, "1000101000001000");
        loadRhythmFromString(0, 0, 4, "1000001000001010");
        loadRhythmFromString(0, 0, 5, "1000101000101000");
        loadRhythmFromString(0, 0, 6, "1000001000101010");
        loadRhythmFromString(0, 0, 7, "1000101000101010");
        // Snare
        loadRhythmFromString(0, 1, 0, "0000100000000000");
        loadRhythmFromString(0, 1, 1, "0000100000100000");
        loadRhythmFromString(0, 1, 2, "0000100000100010");
        loadRhythmFromString(0, 1, 3, "0010100000100010");
        loadRhythmFromString(0, 1, 4, "0011100000100010");
        loadRhythmFromString(0, 1, 5, "0011101000100010");
        loadRhythmFromString(0, 1, 6, "0011101010100010");
        loadRhythmFromString(0, 1, 7, "1011101010101010");
        // Hi-Hat Closed
        loadRhythmFromString(0, 2, 0, "1000000010000000");
        loadRhythmFromString(0, 2, 1, "1000100010001000");
        loadRhythmFromString(0, 2, 2, "1000100010001010");
        loadRhythmFromString(0, 2, 3, "1000101010001010");
        loadRhythmFromString(0, 2, 4, "1000101010101010");
        loadRhythmFromString(0, 2, 5, "1000101010101110");
        loadRhythmFromString(0, 2, 6, "1011101010101110");
        loadRhythmFromString(0, 2, 7, "1011101010101111");
        // Hi-Hat Open
        loadRhythmFromString(0, 3, 0, "0000100000001000");
        loadRhythmFromString(0, 3, 1, "0000100010001000");
        loadRhythmFromString(0, 3, 2, "1000001000001000");
        loadRhythmFromString(0, 3, 3, "1000001000101000");
        loadRhythmFromString(0, 3, 4, "1000001000101010");
        loadRhythmFromString(0, 3, 5, "1000100010001000");
        loadRhythmFromString(0, 3, 6, "0010001000100010");
        loadRhythmFromString(0, 3, 7, "0010001000101010");
        // Perc1
        loadRhythmFromString(0, 4, 0, "0000000000000010");
        loadRhythmFromString(0, 4, 1, "0000000000100010");
        loadRhythmFromString(0, 4, 2, "1000000000100010");
        loadRhythmFromString(0, 4, 3, "1000001000100010");
        loadRhythmFromString(0, 4, 4, "0000101000100010");
        loadRhythmFromString(0, 4, 5, "0010101101101010");
        loadRhythmFromString(0, 4, 6, "0011001110101000");
        loadRhythmFromString(0, 4, 7, "0011011011011000");
        // Perc2
        loadRhythmFromString(0, 5, 0, "0000110000001100");
        loadRhythmFromString(0, 5, 1, "0000110011001100");
        loadRhythmFromString(0, 5, 2, "1010001010001001");
        loadRhythmFromString(0, 5, 3, "1000110011001101");
        loadRhythmFromString(0, 5, 4, "0011001001101011");
        loadRhythmFromString(0, 5, 5, "1010101010001101");
        loadRhythmFromString(0, 5, 6, "1011101011101111");
        loadRhythmFromString(0, 5, 7, "1011101110111001");

            //Style 1: Bulgar #2
        //Kick
        loadRhythmFromString(1, 0, 0, "1000000000000000");
        loadRhythmFromString(1, 0, 1, "0000001000000000");
        loadRhythmFromString(1, 0, 2, "1000000000001000");
        loadRhythmFromString(1, 0, 3, "1000001000000000");
        loadRhythmFromString(1, 0, 4, "1000000010000010");
        loadRhythmFromString(1, 0, 5, "1000001000001000"); 
        loadRhythmFromString(1, 0, 6, "1000001000000010");
        loadRhythmFromString(1, 0, 7, "1000100010001000");
        //Snare
        loadRhythmFromString(1, 1, 0, "0000001000001000");
        loadRhythmFromString(1, 1, 1, "0000001000100000");
        loadRhythmFromString(1, 1, 2, "0000100000100000");
        loadRhythmFromString(1, 1, 3, "0000100000001000");
        loadRhythmFromString(1, 1, 4, "1000001000001000"); 
        loadRhythmFromString(1, 1, 5, "1000001000001001"); 
        loadRhythmFromString(1, 1, 6, "1000100010001000");
        loadRhythmFromString(1, 1, 7, "1000100010001011");
        //Closed Hat
        loadRhythmFromString(1, 2, 1, "1011101010101010");
        loadRhythmFromString(1, 2, 2, "1110101011101010");
        loadRhythmFromString(1, 2, 3, "1001100110011000");
        loadRhythmFromString(1, 2, 4, "1100111011001110");
        loadRhythmFromString(1, 2, 5, "1111111111111111");
        loadRhythmFromString(1, 2, 6, "1000011000000111");
        loadRhythmFromString(1, 2, 7, "1000001010000000"); 
        //Open Hat
        loadRhythmFromString(1, 3, 1, "0010000000100000");
        loadRhythmFromString(1, 3, 2, "0010000000100010");
        loadRhythmFromString(1, 3, 3, "0010001000100000");
        loadRhythmFromString(1, 3, 4, "0010001000100010");
        loadRhythmFromString(1, 3, 5, "1000000010000000");
        loadRhythmFromString(1, 3, 6, "1000000000001000");
        loadRhythmFromString(1, 3, 7, "1000001000001000"); 
        //Perc 1
        loadRhythmFromString(1, 4, 1, "0000100000001000");
        loadRhythmFromString(1, 4, 2, "0000100000100000");
        loadRhythmFromString(1, 4, 3, "0000100010000000");
        loadRhythmFromString(1, 4, 4, "0010000010000000");
        loadRhythmFromString(1, 4, 5, "1000000010000000");
        loadRhythmFromString(1, 4, 6, "1000001000001000");
        loadRhythmFromString(1, 4, 7, "0000100000101000"); 
        //Perc 2
        loadRhythmFromString(1, 5, 1, "0000000010000000");
        loadRhythmFromString(1, 5, 2, "0010000000100000");
        loadRhythmFromString(1, 5, 3, "0001000000100000");
        loadRhythmFromString(1, 5, 4, "1000000000000010");
        loadRhythmFromString(1, 5, 5, "0000100000100010");
        loadRhythmFromString(1, 5, 6, "1100001100001000");
        loadRhythmFromString(1, 5, 7, "0011001100110011");
        
        //Style 2: Araber
        //Kick
        loadRhythmFromString(2, 0, 0, "1000001000001000");
        loadRhythmFromString(2, 0, 1, "1000001010000000");
        loadRhythmFromString(2, 0, 2, "1000001010001000");
        loadRhythmFromString(2, 0, 3, "1000001010001010");
        loadRhythmFromString(2, 0, 4, "1000001010101000");
        loadRhythmFromString(2, 0, 5, "1010001010001000");
        loadRhythmFromString(2, 0, 6, "1010001010001010");
        loadRhythmFromString(2, 0, 7, "1010001010001011");
        //Snare
        loadRhythmFromString(2, 1, 0, "0000000000001000");
        loadRhythmFromString(2, 1, 1, "0000000010000000");
        loadRhythmFromString(2, 1, 2, "0000000000001010");
        loadRhythmFromString(2, 1, 3, "0000100000100000");
        loadRhythmFromString(2, 1, 4, "0000100000000010");
        loadRhythmFromString(2, 1, 5, "0010000000001000");
        loadRhythmFromString(2, 1, 6, "0010000000001011");
        loadRhythmFromString(2, 1, 7, "0010000010001100");
        //Closed Hat
        loadRhythmFromString(2, 2, 0, "0010001000101000");
        loadRhythmFromString(2, 2, 1, "1000110010001100");
        loadRhythmFromString(2, 2, 2, "1110001010001011");
        loadRhythmFromString(2, 2, 3, "1100110011001100");
        loadRhythmFromString(2, 2, 4, "1101101110101000");
        loadRhythmFromString(2, 2, 5, "1011101010101011");
        loadRhythmFromString(2, 2, 6, "1110111011101110");
        loadRhythmFromString(2, 2, 7, "1110111011101111");
        //Open Hat
        loadRhythmFromString(2, 3, 0, "0000100000100000");
        loadRhythmFromString(2, 3, 1, "0000100000001000");
        loadRhythmFromString(2, 3, 2, "0000100000001010");
        loadRhythmFromString(2, 3, 3, "1000000010000000");
        loadRhythmFromString(2, 3, 4, "1000000010001000");
        loadRhythmFromString(2, 3, 5, "1000001010000000");
        loadRhythmFromString(2, 3, 6, "1000001010000010");
        loadRhythmFromString(2, 3, 7, "1000001010000011");
        //Perc 1
        loadRhythmFromString(2, 4, 0, "0000000000001000");
        loadRhythmFromString(2, 4, 1, "0000001000000010");
        loadRhythmFromString(2, 4, 2, "0000100000000010");
        loadRhythmFromString(2, 4, 3, "0000100000100010");
        loadRhythmFromString(2, 4, 4, "0010001000100000");
        loadRhythmFromString(2, 4, 5, "0010001000101000");
        loadRhythmFromString(2, 4, 6, "0010001000100011");
        loadRhythmFromString(2, 4, 7, "0010001000100111");
        //Perc 2 
        loadRhythmFromString(2, 5, 0, "0000000000000010");
        loadRhythmFromString(2, 5, 1, "0000000100000010");
        loadRhythmFromString(2, 5, 2, "0010000000000010");
        loadRhythmFromString(2, 5, 3, "0000101000000000");
        loadRhythmFromString(2, 5, 4, "0010001000000000");
        loadRhythmFromString(2, 5, 5, "0010001000000010");
        loadRhythmFromString(2, 5, 6, "0010001000000011");
        loadRhythmFromString(2, 5, 7, "0010001000000111");
        
        //Style 3: Terkisher
        //Kick
        loadRhythmFromString(3, 0, 0, "1000000000000000");
        loadRhythmFromString(3, 0, 1, "1000000010000000");
        loadRhythmFromString(3, 0, 2, "0000000010001000");
        loadRhythmFromString(3, 0, 3, "1000000010001000");
        loadRhythmFromString(3, 0, 4, "1000001010001000");
        loadRhythmFromString(3, 0, 5, "1000001010001010");
        loadRhythmFromString(3, 0, 6, "1000001010101000");
        loadRhythmFromString(3, 0, 7, "1000001010001001");
        //Snare
        loadRhythmFromString(3, 1, 0, "0000000000001000");
        loadRhythmFromString(3, 1, 1, "0000000010000000");
        loadRhythmFromString(3, 1, 2, "0000100000100000");
        loadRhythmFromString(3, 1, 3, "0000100000000010");
        loadRhythmFromString(3, 1, 4, "1000000010000000");
        loadRhythmFromString(3, 1, 5, "1000001010000000");
        loadRhythmFromString(3, 1, 6, "1000001010001000");
        loadRhythmFromString(3, 1, 7, "1000001010001001");
        //Closed Hat
        loadRhythmFromString(3, 2, 0, "1000001010000010");
        loadRhythmFromString(3, 2, 1, "1000101010000000");
        loadRhythmFromString(3, 2, 2, "1000100010001000");
        loadRhythmFromString(3, 2, 3, "1000101010001000");
        loadRhythmFromString(3, 2, 4, "1000101010001010");
        loadRhythmFromString(3, 2, 5, "1000001010101010");
        loadRhythmFromString(3, 2, 6, "1000101010001100");
        loadRhythmFromString(3, 2, 7, "1000101010001101"); 
        //Open Hat
        loadRhythmFromString(3, 3, 0, "0000000000001000");
        loadRhythmFromString(3, 3, 1, "0000100000001000");
        loadRhythmFromString(3, 3, 2, "1000000000001000");
        loadRhythmFromString(3, 3, 3, "1000000010000000");
        loadRhythmFromString(3, 3, 4, "1000000010001000");
        loadRhythmFromString(3, 3, 5, "1000000010000010");
        loadRhythmFromString(3, 3, 6, "1000001010001000");
        loadRhythmFromString(3, 3, 7, "1000001010001010"); 
        //Perc 1
        loadRhythmFromString(3, 4, 0, "0000100000000000");
        loadRhythmFromString(3, 4, 1, "0000001000000000");
        loadRhythmFromString(3, 4, 2, "0000100000001000");
        loadRhythmFromString(3, 4, 3, "0000001000001000");
        loadRhythmFromString(3, 4, 4, "1000001000000000");
        loadRhythmFromString(3, 4, 5, "1000001010000000");
        loadRhythmFromString(3, 4, 6, "1000001010001000");
        loadRhythmFromString(3, 4, 7, "1000001010001011");
        //Perc 2 
        loadRhythmFromString(3, 5, 0, "0000000000000010");
        loadRhythmFromString(3, 5, 1, "0000000000100000");
        loadRhythmFromString(3, 5, 2, "0000000000001000");
        loadRhythmFromString(3, 5, 3, "0000001000000010");
        loadRhythmFromString(3, 5, 4, "0010000000000000");
        loadRhythmFromString(3, 5, 5, "0010000000000010");
        loadRhythmFromString(3, 5, 6, "0010001000000010"); 
        loadRhythmFromString(3, 5, 7, "0010000000000011"); 

        //Style 4: Hora/Zhok
        //Kick  
        loadRhythmFromString(4, 0, 0, "100000001000");  
        loadRhythmFromString(4, 0, 3, "100000000100");  
        loadRhythmFromString(4, 0, 1, "100000100000");  
        loadRhythmFromString(4, 0, 7, "100000000101");  
        loadRhythmFromString(4, 0, 2, "100000001100");  
        loadRhythmFromString(4, 0, 5, "110000001000");  
        loadRhythmFromString(4, 0, 6, "101000001000");  
        loadRhythmFromString(4, 0, 4, "110000001100");  
        //Snare  
        loadRhythmFromString(4, 1, 0, "100000000000");  
        loadRhythmFromString(4, 1, 1, "100000001000");  
        loadRhythmFromString(4, 1, 5, "000010000100");  
        loadRhythmFromString(4, 1, 4, "000010000001");  
        loadRhythmFromString(4, 1, 2, "000010010000");  
        loadRhythmFromString(4, 1, 3, "000010011000");  
        loadRhythmFromString(4, 1, 7, "001000100010");  
        loadRhythmFromString(4, 1, 6, "100010001000");  
        //Closed Hat  
        loadRhythmFromString(4, 2, 7, "001100110011");  
        loadRhythmFromString(4, 2, 6, "100110011001");  
        loadRhythmFromString(4, 2, 2, "110000001100");  
        loadRhythmFromString(4, 2, 5, "110010101100");  
        loadRhythmFromString(4, 2, 4, "110011001100");  
        loadRhythmFromString(4, 2, 1, "101010101010");  
        loadRhythmFromString(4, 2, 3, "111111111111");  
        //Open Hat  
        loadRhythmFromString(4, 3, 2, "100000000000");  
        loadRhythmFromString(4, 3, 6, "000010000000");  
        loadRhythmFromString(4, 3, 3, "000000100010");  
        loadRhythmFromString(4, 3, 5, "001000000010");  
        loadRhythmFromString(4, 3, 7, "000010001000");  
        loadRhythmFromString(4, 3, 4, "001000100010");  
        loadRhythmFromString(4, 3, 1, "100000001000");  
        //Perc 1  
        loadRhythmFromString(4, 4, 7, "000000100000");  
        loadRhythmFromString(4, 4, 6, "000010000010");  
        loadRhythmFromString(4, 4, 3, "000010001000");  
        loadRhythmFromString(4, 4, 4, "100000001000");  
        loadRhythmFromString(4, 4, 2, "001000100010");  
        loadRhythmFromString(4, 4, 1, "101000001010");  
        loadRhythmFromString(4, 4, 5, "001100000011");  
        //Perc 2  
        loadRhythmFromString(4, 5, 1, "000010000000");  
        loadRhythmFromString(4, 5, 7, "001000000000");  
        loadRhythmFromString(4, 5, 5, "000010001000");  
        loadRhythmFromString(4, 5, 4, "010000000100");  
        loadRhythmFromString(4, 5, 6, "100000100000");  
        loadRhythmFromString(4, 5, 2, "100000001000");  
        loadRhythmFromString(4, 5, 3, "001000100010");          
        //Style 5: In Zibn		
        //Kick		
        loadRhythmFromString(5, 0, 2, "10000001010000");		
        loadRhythmFromString(5, 0, 0, "10001001010000");		
        loadRhythmFromString(5, 0, 7, "10001001001001");		
        loadRhythmFromString(5, 0, 1, "10001001010001");		
        loadRhythmFromString(5, 0, 6, "10001001100100");		
        loadRhythmFromString(5, 0, 5, "10011001100100");		
        loadRhythmFromString(5, 0, 4, "10011001001010");		
        loadRhythmFromString(5, 0, 3, "10101001010100");				
        //Snare		
        loadRhythmFromString(5, 1, 7, "00000000000010");		
        loadRhythmFromString(5, 1, 3, "00001000000001");		
        loadRhythmFromString(5, 1, 5, "00001000000010");		
        loadRhythmFromString(5, 1, 4, "00000100000001");		
        loadRhythmFromString(5, 1, 6, "00100000000001");		
        loadRhythmFromString(5, 1, 0, "00001000000100");		
        loadRhythmFromString(5, 1, 2, "00001001000100");		
        loadRhythmFromString(5, 1, 1, "00001000000101");				
        //Closed Hat		
        loadRhythmFromString(5, 2, 4, "00010000010000");		
        loadRhythmFromString(5, 2, 6, "10000100000010");		
        loadRhythmFromString(5, 2, 3, "00100010001000");		
        loadRhythmFromString(5, 2, 7, "10001100100011");		
        loadRhythmFromString(5, 2, 1, "00110010011001");		
        loadRhythmFromString(5, 2, 2, "10110010011001");		
        loadRhythmFromString(5, 2, 5, "10111011101110");				
        //Open Hat		
        loadRhythmFromString(5, 3, 6, "00000010000001");		
        loadRhythmFromString(5, 3, 2, "00001000000100");		
        loadRhythmFromString(5, 3, 3, "00001000000101");		
        loadRhythmFromString(5, 3, 5, "10001000000001");		
        loadRhythmFromString(5, 3, 4, "10001000100010");		
        loadRhythmFromString(5, 3, 1, "10001001000100");		
        loadRhythmFromString(5, 3, 7, "10101000100101");				
        //Perc 1		
        loadRhythmFromString(5, 4, 3, "00000000000011");		
        loadRhythmFromString(5, 4, 4, "00000010000100");		
        loadRhythmFromString(5, 4, 7, "00010000000100");		
        loadRhythmFromString(5, 4, 6, "00010000000101");		
        loadRhythmFromString(5, 4, 2, "00001100000100");		
        loadRhythmFromString(5, 4, 5, "01000100010001");		
        loadRhythmFromString(5, 4, 1, "00011001100110");				
        //Perc 2 		
        loadRhythmFromString(5, 5, 3, "10000000000000");		
        loadRhythmFromString(5, 5, 5, "00100000000000");		
        loadRhythmFromString(5, 5, 1, "10000001000000");		
        loadRhythmFromString(5, 5, 6, "00000100001000");		
        loadRhythmFromString(5, 5, 4, "00001000001000");		
        loadRhythmFromString(5, 5, 2, "00000000000101");		
        loadRhythmFromString(5, 5, 7, "00100000000001");
    }
};
