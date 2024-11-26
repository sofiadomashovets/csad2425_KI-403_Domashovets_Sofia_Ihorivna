#include <EEPROM.h>

// Поле гри 3x3
char board[3][3];
char currentPlayer = 'X'; // Починає гравець X

String player1 = "P1";
String player2 = "P2";

/**
 * @brief Ініціалізація поля гри 3x3.
 * 
 * Заповнює поле порожніми символами (' ').
 */
void initializeBoard() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      board[i][j] = ' ';
    }
  }
}

/**
 * @brief Виведення поточного стану поля гри у серійний монітор.
 */
void printBoard() {
  Serial.println("Current Board:");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      Serial.print(board[i][j]);
      if (j < 2) Serial.print("|");
    }
    Serial.println();
    if (i < 2) Serial.println("-----");
  }
}

/**
 * @brief Виконання ходу для поточного гравця.
 * 
 * @param row Рядок, у який гравець хоче зробити хід (0-2).
 * @param col Стовпець, у який гравець хоче зробити хід (0-2).
 * @return true Якщо хід успішно виконано.
 * @return false Якщо хід неможливий (поле вже зайняте або координати некоректні).
 */
bool makeMove(int row, int col) {
  if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ') {
    board[row][col] = currentPlayer;
    return true;
  }
  return false;
}

/**
 * @brief Перевірка, чи є переможець на полі.
 * 
 * @return char Символ переможця ('X' або 'O'), або ' ' якщо переможця немає.
 */
char checkWinner() {
  for (int i = 0; i < 3; i++) {
    // Перевірка рядків
    if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ')
      return board[i][0];
    // Перевірка стовпців
    if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ')
      return board[0][i];
  }
  // Перевірка діагоналей
  if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ')
    return board[0][0];
  if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ')
    return board[0][2];
  return ' ';
}

/**
 * @brief Перемикає поточного гравця на іншого.
 */
void switchPlayer() {
  currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
}

/**
 * @brief Читання імен гравців з EEPROM.
 * 
 * Якщо імена не збережені, запитує їх через серійний монітор і зберігає в EEPROM.
 */
void readConfig() {
  char buffer[20];

  // Читання імені гравця 1
  for (int i = 0; i < 20; i++) {
    buffer[i] = EEPROM.read(i);
  }
  player1 = String(buffer);

  // Читання імені гравця 2
  for (int i = 0; i < 20; i++) {
    buffer[i] = EEPROM.read(i + 20);
  }
  player2 = String(buffer);

  // Якщо імена не встановлені, запросити їх через серійний монітор
  if (player1.length() == 0 || player2.length() == 0) {
    Serial.println("Enter Player 1 name:");
    while (!Serial.available());
    player1 = Serial.readStringUntil('\n');
    player1.trim();

    Serial.println("Enter Player 2 name:");
    while (!Serial.available());
    player2 = Serial.readStringUntil('\n');
    player2.trim();

    // Зберегти імена в EEPROM
    for (int i = 0; i < player1.length(); i++) {
      EEPROM.write(i, player1[i]);
    }
    for (int i = player1.length(); i < 20; i++) {
      EEPROM.write(i, '\0');
    }

    for (int i = 0; i < player2.length(); i++) {
      EEPROM.write(i + 20, player2[i]);
    }
    for (int i = player2.length(); i < 20; i++) {
      EEPROM.write(i + 20, '\0');
    }
  }
}

/**
 * @brief Ініціалізація гри та налаштування серійного порту.
 * 
 * Завантажує імена гравців із EEPROM або запитує нові.
 */
void setup() {
  Serial.begin(9600);
  initializeBoard();
  
  // Читання конфігурації з EEPROM
  readConfig();

  Serial.println("Welcome to Tic-Tac-Toe!");
  Serial.println(player1 + " (X) vs " + player2 + " (O)");
}

/**
 * @brief Основний цикл гри.
 * 
 * Дозволяє гравцям по черзі робити ходи, перевіряє переможця та перезапускає гру у разі завершення.
 */
void loop() {
  printBoard();
  Serial.println(String(currentPlayer) + "'s turn. Enter row (0-2) and column (0-2):");

  // Зчитування рядка і стовпця
  int row = -1, col = -1;
  while (row < 0 || row > 2 || col < 0 || col > 2) {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      
      // Розбиваємо введення на рядок та стовпець
      int spaceIndex = input.indexOf(' ');
      if (spaceIndex != -1) {
        row = input.substring(0, spaceIndex).toInt();
        col = input.substring(spaceIndex + 1).toInt();
      } else {
        Serial.println("Invalid input. Please enter row and column as '0 1'.");
      }
    }
    delay(200); // Додаємо невелику затримку для коректного зчитування
  }

  // Виконання ходу та перевірка результату
  if (makeMove(row, col)) {
    char winner = checkWinner();
    if (winner != ' ') {
      printBoard();
      Serial.println(String(winner) + " wins!");
      initializeBoard(); // Скидання гри
    } else {
      switchPlayer();
    }
  } else {
    Serial.println("Invalid move. Try again.");
  }

  delay(1000);  // Пауза перед наступним ходом
}