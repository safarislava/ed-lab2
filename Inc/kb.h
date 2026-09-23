#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7


char Check_Row(uint8_t row);

HAL_StatusTypeDef Set_Keyboard(void);

char Get_Char(void);

#endif /* INC_KEYBOARD_H_ */
