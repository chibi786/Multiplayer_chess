#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <map>
using namespace std;
class Position;
class Queen;
class Knight;
class Bishop;
class Board;
class Piece;
class Rook;
class Player;

Position* final_double_Pawn = nullptr;
class Position {
public:
    int row, col;
    Position(int r = 0, int c = 0) : row(r), col(c) {}
    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
};

class Piece {
protected:
    int color; // 0 = White, 1 = Black
    Position position;
    sf::Sprite sprite;
public:

    Piece(int c, Position p) : color(c), position(p) {}
    void initializeSprite( std::map<std::string, sf::Texture>& textureMap) {
        string p_type = "";
        switch (toupper(getSymbol())) {
        case 'P': p_type = "pawn"; break;
        case 'R': p_type = "rook"; break;
        case 'N': p_type = "knight"; break;
        case 'B': p_type = "bishop"; break;
        case 'Q': p_type = "queen"; break;
        case 'K': p_type = "king"; break;
        default: p_type = "unknown"; break;
        }
        string key = "";
            if (color == 0) {
                key = "white_" + p_type;
            }
            else {
                key = "black_" + p_type;
            }
        sprite.setTexture(textureMap[key]);
        sf::Vector2u textureSize = textureMap[key].getSize();
        float scaleX = 80.0f / textureSize.x;
        float scaleY = 80.0f / textureSize.y;
        sprite.setScale(scaleX, scaleY);
        sprite.setPosition(position.col * 80.0f, position.row * 80.0f); 
    }
    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    void updateSpritePosition() {
        sprite.setPosition(position.col * 80, position.row * 80);
    }
    virtual ~Piece() {}
    int get_color() const { return color; }
    Position get_position() const { return position; }
    void set_position(Position p) {
        position = p;
        updateSpritePosition();
    }
    virtual bool move_to_newloc(Position to, Piece** board[8]) = 0;
    virtual char getSymbol() const = 0;
};

class Rook : public Piece {
private:
    bool has;
public:
    Rook(int c, Position p) : Piece(c, p), has(false) {}

    bool move_to_newloc(Position change, Piece*** board) override {
        if (change.row != position.row && change.col != position.col) { return false; }
        int c_r = (change.row > position.row) ? 1 : (change.row < position.row) ? -1 : 0;
        int c_c = (change.col > position.col) ? 1 : (change.col < position.col) ? -1 : 0;

        int r = position.row + c_r;
        int c = position.col + c_c;

        while (r != change.row || c != change.col) {
            if (board[r][c] != nullptr) return false;
            r += c_r;
            c += c_c;
        }

        if (board[change.row][change.col] != nullptr && board[change.row][change.col]->get_color() == color) return false;

        if (board[change.row][change.col] != nullptr) delete board[change.row][change.col];

        board[change.row][change.col] = this;
        board[position.row][position.col] = nullptr;
        position = change;
        has = true;
        return true;
    }
    char getSymbol() const override {
        if (color == 0)
        {
            return 'R';
        }
        else {
            return 'r';
        }
    }
    bool has_moved() const { return has; }
    void set_moved() { has = true; }
};

class Knight : public Piece {
public:
    Knight(int c, Position p) : Piece(c, p) {}

    bool move_to_newloc(Position change, Piece*** board) override {
        int c_r = abs(change.row - position.row), c_c = abs(change.col - position.col);
        if ((c_r == 2 && c_c == 1) || (c_r == 1 && c_c == 2)) {
            Piece* target = board[change.row][change.col];
            if (target && target->get_color() == color) {
                return false; // Cannot capture own piece
            }

            if (target) delete target; // Capture opponent piece

            board[change.row][change.col] = this;
            board[position.row][position.col] = nullptr;
            position = change;
            return true;
        }
        return false;
    }  char getSymbol() const override {
        if (color == 0)
        {
            return 'N';
        }
        else
        {
            return 'n';
        }
    }
};

class Bishop : public Piece {
public:
    Bishop(int c, Position p) : Piece(c, p) {}

    bool move_to_newloc(Position change, Piece** board[8]) override {
        int c_r = change.row - position.row;
        int c_c = change.col - position.col;
        if (abs(c_r) != abs(c_c)) { return false; }

        c_r = c_r / abs(c_r); // Normalize direction
        c_c = c_c / abs(c_c);

        int r = position.row + c_r, c = position.col + c_c;
        while (r != change.row || c != change.col) {
            if (board[r][c] != nullptr) { return false; }
            r += c_r; c += c_c;
        }

        Piece* target = board[change.row][change.col];
        if (target && target->get_color() == color) { return false; }
        if (target) { delete target; }

        board[change.row][change.col] = this;
        board[position.row][position.col] = nullptr;
        position = change;
        return true;
    }

    char getSymbol() const override {
        if (color == 0)
        {
            return 'B';
        }
        else
        {
            return 'b';
        }
    }
};

class Queen : public Piece {
public:
    Queen(int c, Position p) : Piece(c, p) {}

    bool move_to_newloc(Position change, Piece** board[8]) override {
        int c_r = change.row - position.row;
        int c_c = change.col - position.col;

        if (c_r != 0 && c_c != 0 && abs(c_r) != abs(c_c)) return false;

        if (c_r == 0) {
            c_r = 0;
        }
        else {
            c_r = c_r / abs(c_r);
        }

        if (c_c == 0) {
            c_c = 0;
        }
        else {
            c_c = c_c / abs(c_c);
        }


        int r = position.row + c_r, c = position.col + c_c;
        while (r != change.row || c != change.col) {
            if (board[r][c] != nullptr) { return false; }
            r += c_r; c += c_c;
        }

        Piece* target = board[change.row][change.col];
        if (target && target->get_color() == color) { return false; }
        if (target) { delete target; }

        board[change.row][change.col] = this;
        board[position.row][position.col] = NULL;
        position = change;
        return true;
    }

    char getSymbol() const override {
        if (color == 0)
        {
            return 'Q';
        }
        else
        {
            return 'q';
        }
    }
};

class King;
class Pawn;

class Board {
public:
    Piece*** board; 
    Board() {
        board = new Piece * *[8];
        for (int i = 0; i < 8; ++i) {
            board[i] = new Piece * [8];
            for (int j = 0; j < 8; ++j)
                board[i][j] = nullptr;
        }

    }
    void display(sf::RenderWindow& window) {

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                sf::RectangleShape square(sf::Vector2f(80.f, 80.f));
                square.setPosition(col * 80, row * 80);

                if ((row + col) % 2 == 0)
                    square.setFillColor(sf::Color(240, 217, 181)); 
                else
                    square.setFillColor(sf::Color(181, 136, 99));  

                window.draw(square);
            }
        }
      for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if (board[row][col])
                    board[row][col]->draw(window);
            }
        }
    }
    ~Board() {
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (board[i][j] != nullptr)
                    delete board[i][j];
            }
            delete[] board[i];
        }
        delete[] board;
    }

    static bool check_if_terror(Position pos, int byColor, Piece** board[8]) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece* predator = board[i][j];
                if (predator && predator->get_color() == byColor) {
                    Position from(i, j);
                    // Save original position
                    Position oldPos = predator->get_position();

                    // Temporarily place attacker at its position to avoid nullptr issues in moveTo
                    predator->set_position(from);

                    // Do a fake move test (no actual board change)
                    if (predator->move_to_newloc(pos, board)) {
                        // Revert position
                        predator->set_position(oldPos);
                        return true;
                    }

                    // Revert position if changed
                    predator->set_position(oldPos);
                }
            }
        }
        return false;
    }

    Piece* getPieceAt(Position loc) const {
        if (!loc.isValid()) { return nullptr; }
        return board[loc.row][loc.col];
    }
};


class King : public Piece {
    bool hasMoved = false;

public:
    King(int c, Position p) : Piece(c, p) {}

    bool move_to_newloc(Position change, Piece** board[8]) override {
        int c_r = change.row - position.row;
        int c_c = change.col - position.col;

        // Standard one-square move
        if (abs(c_r) <= 1 && abs(c_c) <= 1 && (c_r != 0 || c_c != 0)) {
            Piece* target = board[change.row][change.col];
            if (target != NULL && target->get_color() == color) {
                return false;
            }

            // Simulate move to check for self-check
            Piece* from = board[position.row][position.col];
            Piece* to = board[change.row][change.col];

            board[position.row][position.col] = NULL;
            board[change.row][change.col] = this;

            bool inCheck = Board::check_if_terror(change, 1 - color, board);

            // Undo the simulated move
            board[position.row][position.col] = from;
            board[change.row][change.col] = to;

            if (inCheck) return false;

            if (target != NULL) {
                delete target;
            }

            board[change.row][change.col] = this;
            board[position.row][position.col] = NULL;
            position = change;
            hasMoved = true;
            return true;
        }

        // Castling
        if (!hasMoved && c_r == 0 && (c_c == 2 || c_c == -2)) {
            int rookCol;
            if (c_c > 0) {
                rookCol = 7; // kingside
            }
            else {
                rookCol = 0; // queenside
            }

            Piece* rook = board[position.row][rookCol];
            if (rook == NULL) return false;

            char rookSymbol = rook->getSymbol();
            if (color == 0 && rookSymbol != 'R') return false;
            if (color == 1 && rookSymbol != 'r') return false;

            int direction;
            if (c_c > 0) {
                direction = 1;
            }
            else {
                direction = -1;
            }

            // Path between king and rook must be clear
            for (int c = position.col + direction; c != rookCol; c += direction) {
                if (board[position.row][c] != NULL) return false;
            }

            // Cannot castle through or into check
            for (int c = position.col; c != change.col + direction; c += direction) {
                if (Board::check_if_terror(Position(position.row, c), 1 - color, board)) return false;
            }

            // Move rook
            int newRookCol;
            if (c_c > 0) {
                newRookCol = 5;
            }
            else {
                newRookCol = 3;
            }

            board[position.row][newRookCol] = rook;
            board[position.row][rookCol] = NULL;
            rook->set_position(Position(position.row, newRookCol));

            // Move king
            board[change.row][change.col] = this;
            board[position.row][position.col] = NULL;
            position = change;
            hasMoved = true;
            return true;
        }

        return false;
    }

    char getSymbol() const override {
        if (color == 0) {
            return 'K';
        }
        else {
            return 'k';
        }
    }
};


class Pawn : public Piece {
public:
    Pawn(int c, Position p) : Piece(c, p) {}

    bool move_to_newloc(Position change, Piece** board[8]) override {
        int direction_update;
        if (color == 0) {
            direction_update = -1;
        }
        else {
            direction_update = 1;
        }
        int startRow;
        if (color == 0) {
            startRow = 6;
        }
        else {
            startRow = 1;
        }
        // Regular move
        if (change.col == position.col && board[change.row][change.col] == NULL) {
            if (change.row == position.row + direction_update) {
                // Move
                board[change.row][change.col] = this;
                board[position.row][position.col] = NULL;
                position = change;

                // Clear en passant if set
                if (final_double_Pawn) {
                    delete final_double_Pawn;
                    final_double_Pawn = nullptr;
                }

                // Promotion
                if (change.row == 0 || change.row == 7) {
                    delete board[change.row][change.col];
                    board[change.row][change.col] = new Queen(color, change);
                }

                return true;
            }
            // Double-step move from starting position
            else if (position.row == startRow && change.row == position.row + 2 * direction_update &&
                board[position.row + direction_update][change.col] == NULL) {
                board[change.row][change.col] = this;
                board[position.row][position.col] = NULL;
                position = change;

                // Set en passant target
                if (final_double_Pawn) { delete final_double_Pawn; }
                final_double_Pawn = new Position(change.row, change.col);

                return true;
            }
        }

        // Normal diagonal capture
        else if (abs(change.col - position.col) == 1 && change.row == position.row + direction_update) {
            Piece* target = board[change.row][change.col];
            if (target && target->get_color() != color) {
                delete target;
                board[change.row][change.col] = this;
                board[position.row][position.col] = NULL;
                position = change;

                if (final_double_Pawn) {
                    delete final_double_Pawn;
                    final_double_Pawn = nullptr;
                }

                // Promotion
                if (change.row == 0 || change.row == 7) {
                    delete board[change.row][change.col];
                    board[change.row][change.col] = new Queen(color, change);
                }

                return true;
            }

            // En Passant
            if (final_double_Pawn && final_double_Pawn->row == position.row &&
                final_double_Pawn->col == change.col) {
                Piece* en_p_target = board[position.row][change.col];
                if (en_p_target && en_p_target->get_color() != color &&
                    dynamic_cast<Pawn*>(en_p_target)) {
                    // Capture
                    delete en_p_target;
                    board[position.row][change.col] = NULL;

                    board[change.row][change.col] = this;
                    board[position.row][position.col] = NULL;
                    position = change;
                    delete final_double_Pawn;
                    final_double_Pawn = nullptr;
                    return true;
                }
            }
        }

        return false;
    }
    char getSymbol() const override {
        if (color == 0) {
            return 'P';
        }
        else {
            return 'p';
        }
    }
};


class Player {
private:
    int color; // 0 = White, 1 = Black
    time_t start_time;
    time_t total_time;
    Board* board;

    Position findKingPosition() {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece* piece = board->board[i][j];
                if (piece && piece->get_color() == color) {
                    char sym = piece->getSymbol();
                    if ((color == 0 && sym == 'K') || (color == 1 && sym == 'k')) {
                        return Position(i, j);
                    }
                }
            }
        }
        return Position(-1, -1);
    }

public:
    Player(int c, Board* b) : color(c), board(b), total_time(120) {
        start_time = time(0);
    }

    int getcolor() {
        return color;
    }

    void start_turn() {
        start_time = time(0);
    }

    void end_turn() {
        time_t end_time = time(0);
        total_time = total_time - difftime(end_time, start_time);

        if (total_time <= 0) {
            sf::RenderWindow gameOverWindow(sf::VideoMode(640, 720), "Game Over");
            sf::Font font;
            if (!font.loadFromFile("arial.ttf")) {
                std::cerr << "Failed to load font!\n";
                exit(1);
            }

            std::string message;
            if (color == 0) {
                message = "Time's up!\nWhite Player Lost!";
            }
            else {
                message = "Time's up!\nBlack Player Lost!";
            }

            sf::Text gameOverText(message, font, 48);
            gameOverText.setFillColor(sf::Color::Red);
            gameOverText.setPosition(60, 300);

            sf::Text exitPrompt("Press any key to exit...", font, 28);
            exitPrompt.setFillColor(sf::Color::White);
            exitPrompt.setPosition(120, 400);

            while (gameOverWindow.isOpen()) {
                sf::Event event;
                while (gameOverWindow.pollEvent(event)) {
                    if (event.type == sf::Event::Closed)
                        gameOverWindow.close();
                    if (event.type == sf::Event::KeyPressed)
                        exit(0); 
                }

                gameOverWindow.clear(sf::Color::Black);
                gameOverWindow.draw(gameOverText);
                gameOverWindow.draw(exitPrompt);
                gameOverWindow.display();
            }
        }

        if (color == 0) {
            std::cout << "Time remaining for Player White: " << total_time << " seconds\n";
        }
        else {
            std::cout << "Time remaining for Player Black: " << total_time << " seconds\n";
        }
    }


    bool isCheck() {
        Position king_position = findKingPosition();
        return Board::check_if_terror(king_position, 1 - color, board->board);
    }

    bool isCheckmate() {
        if (!isCheck()) return false;

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece* piece = board->board[i][j];
                if (piece && piece->get_color() == color) {
                    Position from = piece->get_position();
                    for (int r = 0; r < 8; r++) {
                        for (int c = 0; c < 8; c++) {
                            Position to(r, c);
                            if (!to.isValid()) continue;
                            Piece* target = board->board[r][c];
                            if (target && target->get_color() == color) continue;
                            Piece* backup = board->board[r][c];
                            board->board[from.row][from.col] = NULL;
                            board->board[r][c] = piece;
                            Position oldPos = piece->get_position();
                            piece->set_position(to);
                            bool stillInCheck = Board::check_if_terror(findKingPosition(), 1 - color, board->board);
                            board->board[from.row][from.col] = piece;
                            board->board[r][c] = backup;
                            piece->set_position(oldPos);

                            if (!stillInCheck) { return false; }
                        }
                    }
                }
            }
        }

        return true;
    }

    bool isStalemate() {
        if (isCheck()) return false;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece* piece = board->board[i][j];
                if (piece && piece->get_color() == color) {
                    Position from = piece->get_position();
                    for (int r = 0; r < 8; r++) {
                        for (int c = 0; c < 8; c++) {
                            Position to(r, c);
                            if (!to.isValid()) continue;
                            Piece* target = board->board[r][c];
                            if (target && target->get_color() == color) continue;
                            Piece* backup = board->board[r][c];
                            board->board[from.row][from.col] = NULL;
                            board->board[r][c] = piece;
                            Position oldPos = piece->get_position();
                            piece->set_position(to);
                            bool stillInCheck = Board::check_if_terror(findKingPosition(), 1 - color, board->board);
                            board->board[from.row][from.col] = piece;
                            board->board[r][c] = backup;
                            piece->set_position(oldPos);
                            if (!stillInCheck) return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    int getRemainingTime() {
        return total_time - difftime(time(0), start_time);
    }
};


int main() {
    sf::RenderWindow window(sf::VideoMode(640, 640), "Chess Game");
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return 1;
    }
    map<std::string, sf::Texture> textureMap;

    string pieces[] = { "pawn", "rook", "knight", "bishop", "queen", "king" };
    string colors[] = { "white", "black" };

    for (const string& color : colors) {
        for (const string& piece : pieces) {
            string filename = "assets/" + color + "_" + piece + ".png";
            sf::Texture tex;
            if (!tex.loadFromFile(filename)) {
                std::cerr << "Failed to load " << filename << "\n";
                return 1;
            }
            textureMap[color + "_" + piece] = tex;
        }
    }

    sf::Text introText("Chess Game\nPress Enter to Play\n Rules: \nFirst touch to choose piece \nand second to move\nEach player has a time\n of 120 seconds, use wisely", font, 36);
    introText.setFillColor(sf::Color::Black);
    introText.setPosition(80, 250);
    window.clear(sf::Color::White);
    window.draw(introText);
    window.display();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
                goto start_game;
        }
    }

start_game:
    Board board;

    board.board[0][0] = new Rook(1, Position(0, 0));
    board.board[0][1] = new Knight(1, Position(0, 1));
    board.board[0][2] = new Bishop(1, Position(0, 2));
    board.board[0][3] = new Queen(1, Position(0, 3));
    board.board[0][4] = new King(1, Position(0, 4));    
    board.board[0][5] = new Bishop(1, Position(0, 5));
    board.board[0][6] = new Knight(1, Position(0, 6));
    board.board[0][7] = new Rook(1, Position(0, 7));
    for (int j = 0; j < 8; ++j)
    {
        board.board[1][j] = new Pawn(1, Position(1, j));
    }

    board.board[7][0] = new Rook(0, Position(7, 0));
    board.board[7][1] = new Knight(0, Position(7, 1));
    board.board[7][2] = new Bishop(0, Position(7, 2));
    board.board[7][3] = new Queen(0, Position(7, 3));
    board.board[7][4] = new King(0, Position(7, 4));    
    board.board[7][5] = new Bishop(0, Position(7, 5));
    board.board[7][6] = new Knight(0, Position(7, 6));
    board.board[7][7] = new Rook(0, Position(7, 7));
    for (int j = 0; j < 8; ++j)
    {
        board.board[6][j] = new Pawn(0, Position(6, j));
    }

    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            if (board.board[i][j])
            {
                board.board[i][j]->initializeSprite(textureMap);
            }
        }
    }

    Player white(0, &board);
    Player black(1, &board);
    bool whiteTurn = true;
    Position selected(-1, -1);
    bool piece_selected = false;

    white.start_turn();
   
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int col = event.mouseButton.x / 80;
                int row = event.mouseButton.y / 80;
                Position clicked(row, col);

                if (!piece_selected) {
                    Piece* p = board.getPieceAt(clicked);
                    if (p) {
                        int current_t_c;
                        if (whiteTurn)
                            current_t_c = 0;
                        else
                            current_t_c = 1;

                        if (p->get_color() == current_t_c) {
                            selected = clicked;
                            piece_selected = true;
                        }
                    }
                }
                else {
                    Piece* p = board.getPieceAt(selected);
                    if (p && p->move_to_newloc(clicked, board.board)) {
                        if (whiteTurn)
                            white.end_turn();
                        else
                            black.end_turn();

                        whiteTurn = !whiteTurn;
                        if (whiteTurn)
                            white.start_turn();
                        else
                            black.start_turn();
                    }
                    piece_selected = false;
                }
            }
        }

        window.clear();
        board.display(window);
        window.display();

        if (whiteTurn && white.isCheckmate()) {
            std::cout << "Checkmate! Black wins!\n";
            window.close();
        }
        else if (!whiteTurn && black.isCheckmate()) {
            std::cout << "Checkmate! White wins!\n";
            window.close();
        }
        else if (whiteTurn && white.isStalemate()) {
            std::cout << "Stalemate! It's a draw.\n";
            window.close();
        }
        else if (!whiteTurn && black.isStalemate()) {
            std::cout << "Stalemate! It's a draw.\n";
            window.close();
        }
    }

    return 0;
}
