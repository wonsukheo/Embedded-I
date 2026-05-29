#include "CommandTerminalInterface.h"

int myStrCmp(const char* str1, const char* str2)
{
	const char* p1 = str1;
	const char* p2 = str2;
	
	while (*p1 != '\0' && *p1 == *p2) {
		p1++;
		p2++;
	}
	
	return (*p1 - *p2 == 0) ? 1 : 0;
}

int myStrLen(const char* str)
{
	const char* p1 = str;
	const char* p2 = p1;
		
	while (*p2++ != '\0');
	
	return p2 - p1;
}

char* myStrCpy(const char* original, char* copy)
{
	const char* p1 = original;
	const char* p2 = copy;

	while (*p1 != '\0') {
		*p2++ = *p1++;
        }

	*p2 = '\0';

	return copy;
}

uint8_t getFieldCount(USER_DATA* data) 
{
    return data->fieldCount;
}
// take buffer str from getsUart0()
// process string in-place and return field info
void parseFields(USER_DATA* data) 
{
	uint32_t i;
	uint8_t prevField;
	bool commandFlag = 1;
	char readChar;
	
	for (i = 0; i < MAX_CHARS; i++) {
		readChar = data->buffer[i];
		
		if (readChar == '\0') return;

		if (readChar >='a' && readChar <= 'z') {
			if (prevField != 'a') {
				data->fieldType[data->fieldCount] = 'a';
				data->fieldPosition[data->fieldCount++] = i;
				prevField = 'a';
			}			
		}
//hypen, comma, period off
		else if (readChar >= '0' && readChar <= '9') {
			if (prevField != 'n') {
				data->fieldType[data->fieldCount] = 'n';
				data->fieldPosition[data->fieldCount++] = i;
				prevField = 'n';
			}	
		}
//convert delimeter to NULL
		else {
			if (commandFlag) {
				data->buffer[i] = '\0';	
				commandFlag = 0;
			}
					
			prevField = 'd';
		}
	}		
	
	return;
}

char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
	if (data->fieldCount < fieldNumber) {
        return '\0';
    } 
        
	int i = data->fieldPosition[fieldNumber-1];
        
    return &(data->buffer[i]);
}

int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{
	if (data->fieldCount < fieldNumber) {
        return '\0';
    } 
    
    if (data->fieldType[fieldNumber] == 'n') {
    	int i = data->fieldPosition[fieldNumber-1];
    	
    	return data->buffer[i];
    }
    
	return 0;
}

bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{	
	char* str = getFieldString(data, 1);
	
	if (myStrCmp(str, strCommand)) {
		if (data->fieldCount >= minArguments) return 1;
	}
	
	return 0;	
}

