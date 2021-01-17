// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

// I could probably have made this a struct. I forgot they existed because Python.
class Vect2 {
public:
    // Just a container for two values.
    int x;
    int y;

    Vect2(int ax, int ay) {
        x = ax;
        y = ay;
    }

    Vect2 operator+(const Vect2& v) {
        return Vect2(x + v.x, y + v.y);
    }
};


class Tetromino {
public:
    // Maybe I could package these variables into a struct.
    bool isNull = false;
    vector<wstring> shape;
    int ascii_index; // This what index in the ascii symbol array the piece will use when displayed. Not a good system, should be replaced when I rework this.
    int pieceGridSize; // This is used for rotation, to determine where the origin point of the piece is.
    int shapeNum;
    Vect2 pos = Vect2(0, 0);

    // First dimension is the block, second is y position, third is x position.
    // 0-6, pieces are: i, o, l, j, s, z, t
    static vector<vector<wstring>> tetrominos;

    Tetromino(int aShape) {
        if (aShape == -1) {
            isNull = true;
            return;
        }

        shapeNum = aShape;
        shape = tetrominos[shapeNum]; // Get a copy of the shape that's being requested.
        if (shapeNum == 0)
            pieceGridSize = 4;
        else if (shapeNum == 1) // Square doesn't need rotating.
            pieceGridSize = 0;
        else
            pieceGridSize = 3;
        ascii_index = aShape + 1;
        pos = Vect2(0, 0);
    }

    vector<wstring> RotateLeft() {
        return Rotate(shape, pieceGridSize, false);
    }

    vector<wstring> RotateRight() {
        return Rotate(shape, pieceGridSize, true);
    }

    static vector<wstring> Rotate(vector<wstring> aPiece, int size, bool clockwise = true) {
        /// <summary>
        /// Rotates a piece.
        /// aPiece: The piece to rotate.
        /// size: The size of the array to rotate. This can be smaller than the vector's size to get a different origin point.
        /// clockwise: Direction to rotate.
        /// </summary>
        vector<wstring> rotated = aPiece;
        
        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                if (clockwise)
                    rotated[x][y] = aPiece[size - y - 1][x]; // Transpose, and reverse the array.
                else
                    rotated[x][y] = aPiece[y][size - x - 1];
            }
        }

        return rotated;
    }

    wchar_t ShapeCell(int x, int y) {
        return shape[y][x];
    }
    
    static wchar_t ShapeCell(vector<wstring> shape, int x, int y) {
        return shape[y][x];
    }
};

// I don't quite get why C++ requires this weird cemantic way of declaring static variables.
vector<vector<wstring>> Tetromino::tetrominos = {
        { // I piece
            L"....",
            L"XXXX",
            L"....",
            L"...."
        },
        { // O piece
            L".XX.",
            L".XX.",
            L"....",
            L"...."
        },
        { // L piece
            L"..X.",
            L"XXX.",
            L"....",
            L"...."
        },
        { // J piece
            L"X...",
            L"XXX.",
            L"....",
            L"...."
        },
        { // S piece
            L".XX.",
            L"XX..",
            L"....",
            L"...."
        },
        { // Z piece
            L"XX..",
            L".XX.",
            L"....",
            L"...."
        },
        { // T piece
            L".X..",
            L"XXX.",
            L"....",
            L"...."
        }
};


class TetrisField {
public:
    Vect2 size = Vect2(0,0); // Width and height of the field.
    Vect2 padding = Vect2(0, 0);
    Vect2 pieceSpawn = Vect2(4, 0); // Where pieces spawn on the board.
    unsigned char* pField = nullptr;

    TetrisField(int aWidth, int aHeight, int aXPad, int aYPad) {
        size = Vect2(aWidth, aHeight);
        padding = Vect2(aXPad, aYPad);

        pField = new unsigned char[size.x * size.y];
        for (int x = 0; x < size.x; x++) {
            for (int y = 0; y < size.y; y++) {
                int pos = y * size.x + x;
                // If the position is the left, right, or bottom wall, set it to the wall value. Else, set it to empty.
                if (x == 0 || x == size.x - 1 || y == size.y - 1) pField[pos] = 9;
                else pField[pos] = 0;
            }
        }
    }

    int pos(int x, int y) {
        return pField[y * size.x + x];
    }

    int pos(int index) {
        return pField[index];
    }

    void ClearLine() {

    }

    void AddPiece(Tetromino piece) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int pos = ((piece.pos.y + i) * size.x) + piece.pos.x + j;
                if (piece.ShapeCell(j, i) == *L"X")
                    pField[pos] = piece.ascii_index;
            }
        }
    }
};


class PieceBag {
public:
    vector<Tetromino> currentBag = {0, 1, 2, 3, 4, 5, 6}; // Creates an array containing one of each piece.
    vector<Tetromino> nextBag = {0, 1, 2, 3, 4, 5, 6}; // Creates an array containing one of each piece.
    int bagIndex = -1; // Initialized to -1 so the first use will put it at index 0.

    PieceBag() {
        shuffle(std::begin(currentBag), std::end(currentBag), default_random_engine(std::rand()));
        shuffle(std::begin(nextBag), std::end(nextBag), default_random_engine(std::rand()));
    }

    Tetromino NextPiece() {
        // Increment the piece bag.
        bagIndex++;
        if (bagIndex == 7) NextBag(); // Loop to the next bag.
        return currentBag[bagIndex];
    }

private:
    void NextBag() {
        bagIndex = 0;
        currentBag = nextBag;
        nextBag = {0, 1, 2, 3, 4, 5, 6};
        shuffle(std::begin(nextBag), std::end(nextBag), default_random_engine(std::rand()));
    }
};


bool DoesPieceFit(TetrisField field, vector<wstring> shape, Vect2 pos, Vect2 posoffset = Vect2(0, 0)) {
    int piece_size = shape.size();
    pos = pos + posoffset;
    for (int x = 0; x < piece_size; x++) {
        for (int y = 0; y < piece_size; y++) {
            int fi = (pos.y + y) * field.size.x + (pos.x + x);

            // Collision detection
            if (pos.x + x >= 0 && pos.x + x < field.size.x) { // Within bounds of field on x
                if (pos.y + y >= 0 && pos.y + y < field.size.y) { // Within bounds on y.
                    if (Tetromino::ShapeCell(shape, x, y) == *L"X" && field.pos(fi) != 0) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
};


class TetrisController {
// An object that handles all the input a player can do.
public:
    // Codes: space, A, W, D, S, left arrow, right arrow.
    char keyBinds[8] = "\x20\AWDS\x25\x27";

    Tetromino currentPiece = Tetromino(-1);
    TetrisField field = TetrisField(12, 20, 5, 3);

    PieceBag pieceBag = PieceBag();

    bool gameOver = false;

    bool inputArray[7];
    bool *holdPiece = &inputArray[0];
    bool *moveLeft = &inputArray[1];
    bool *moveRight = &inputArray[3];
    bool *hardDrop = &inputArray[2];
    bool *softDrop = &inputArray[4];
    bool *rotateLeft = &inputArray[5];
    bool *rotateRight = &inputArray[6];

    bool hardDropHold = false;
    bool rotateHoldRight = false;
    bool rotateHoldLeft = false;

    // Tick trackers.
    const int lockDelay = 20; // How many times the decrement has to fail before forcing the piece to lock.
    int lockCounter = 0; // How many times the "lock" has failed.

    int speed = 20; // How many ticks need to pass for the piece to descend. 
    int speedCounter = 0; // How many ticks have passed since the last descent.

    void Update() {
        if (currentPiece.isNull) {
            NextPiece();
        }
        // Check all keybinds to see what was pressed.
        for (int k = 0; k < size(keyBinds); k++) {
            inputArray[k] = (0x8000 & GetAsyncKeyState((unsigned char)(keyBinds[k]))) != 0;
        }

        speedCounter++;


        if (*hardDrop) {
            if (!hardDropHold) {
                HardDrop();
            }
            hardDropHold = true;
        }
        else hardDropHold = false;

        if (*moveLeft && DoesPieceFit(field, currentPiece.shape, currentPiece.pos, Vect2(-1, 0)))
            currentPiece.pos.x -= 1;
        
        if (*moveRight && DoesPieceFit(field, currentPiece.shape, currentPiece.pos, Vect2(1, 0)))
            currentPiece.pos.x += 1;

        if (*softDrop || speedCounter >= speed)
            SoftDrop();

        // TODO: Implement kicking
        if (*rotateLeft) {
            if (!rotateHoldLeft && DoesPieceFit(field, currentPiece.RotateLeft(), currentPiece.pos)) {
                currentPiece.shape = currentPiece.RotateLeft();
            }
            rotateHoldLeft = true;
        }
        else rotateHoldLeft = false;

        if (*rotateRight) {
            if (!rotateHoldRight && DoesPieceFit(field, currentPiece.RotateRight(), currentPiece.pos)) {
                currentPiece.shape = currentPiece.RotateRight();
            }
            rotateHoldRight = true;
        }
        else rotateHoldRight = false;

        // Check if there is a collision directly below the piece. If so, the piece will increase the "lock" counter.
        if (!DoesPieceFit(field, currentPiece.shape, currentPiece.pos, Vect2(0, 1))) {
            lockCounter++;
        }

        // Piece has been against the ground long enough to force it to lock.
        if (lockCounter >= lockDelay)
            LockPiece();
    }


    void HardDrop() {
        bool inAir = true;
        while (inAir) {
            inAir = SoftDrop();
        }
        LockPiece();
    }


    bool SoftDrop() {
        speedCounter = 0;
        if (DoesPieceFit(field, currentPiece.shape, currentPiece.pos, Vect2(0, 1))) {
            currentPiece.pos.y += 1;
            return true;
        }
        else return false;
    }

    // TODO: These.
    void NextPiece() {
        currentPiece = pieceBag.NextPiece();
        currentPiece.pos = field.pieceSpawn;
    }

    void LockPiece() {
        lockCounter = 0;
        speedCounter = 0;
        // Add piece to the field.
        field.AddPiece(currentPiece);

        field.ClearLine();

        // Get next piece
        NextPiece();

        // If pieces doesn't fit, game lost.
        if (!DoesPieceFit(field, currentPiece.shape, currentPiece.pos)){
            gameOver = true;
        }
    }
};


int main()
{
    std::srand(11); // Initialize seed for random generation.
    TetrisController c = TetrisController();

    wchar_t asciiSub[] = L" IOLJSZT=#"; // Each grid space is given a number. This number is substituted for a character in this string when rendering.
    Vect2 screenSize = Vect2(80, 30);
    
    // Create Screen Buffer
    wchar_t *screen = new wchar_t[screenSize.x * screenSize.y];
    for (int i = 0; i < screenSize.x * screenSize.y; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, 3);
    DWORD dwBytesWritten = 0;
    WORD color = 15;
    COORD batchStartPos;

    while (!c.gameOver) {
        // Game timing
        this_thread::sleep_for(50ms); // A game tick happens this often.

        // Game logic
        c.Update();

        // ========== Render output ==========

        // Draw the field.
        for (int x = 0; x < c.field.size.x; x++) {
            for (int y = 0; y < c.field.size.y; y++) {
                // Checks a position in pField for the related number,
                // uses that number to select from this string,
                // and then draws the character to screen.
                int pos = (y + c.field.padding.y) * screenSize.x + (x + c.field.padding.x); // Position on the screen to draw to. Offset by 2 on x and y.
                screen[pos] = asciiSub[c.field.pos(x, y)];
            }
        }

        // Draw current piece
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                if (c.currentPiece.ShapeCell(x, y) == *L"X") {
                    int pos = (y + c.currentPiece.pos.y + c.field.padding.y) * screenSize.x + (x + c.currentPiece.pos.x + c.field.padding.x); // Position on the screen to draw to. Offset by 2 on x and y.
                    screen[pos] = asciiSub[c.currentPiece.ascii_index];
                }
            }
        }

        // TODO Draw a ghost of the piece.

        // TODO Draw upcoming

        // TODO Draw score

        // TODO Draw hold

        // Draw to console.
        WriteConsoleOutputCharacter(hConsole, screen, screenSize.x * screenSize.y, { 0, 0 }, &dwBytesWritten);

        // Draw colour on the field.

        // Group up like characters, and draw them in a batch.
        int repeatCounter = 0;
        wchar_t lastCharacter = *L"";
        bool initbatch = true;
        for (int x = 0; x < c.field.size.x; x++) {
            for (int y = 0; y < c.field.size.y; y++) {
                int pos = (y + c.field.padding.y) * screenSize.x + (x + c.field.padding.x);
                wchar_t symbol = screen[pos];

                if (initbatch) {
                    repeatCounter = 1;
                    lastCharacter = symbol;
                    batchStartPos.X = x + c.field.padding.x;
                    batchStartPos.Y = y + c.field.padding.y;
                    initbatch = false;
                    continue;
                }

                if (symbol == lastCharacter) {
                    repeatCounter++;
                    continue;
                }

                else {
                    switch (symbol) {
                    case * L"I": color = 11; break;
                    case * L"O": color = 14; break;
                    case * L"J": color = 19; break;
                    case * L"L": color = 6; break;
                    case * L"T": color = 13; break;
                    case * L"Z": color = 12; break;
                    case * L"S": color = 10; break;
                    default: color = 15;
                    }
                    WriteConsoleOutputAttribute(hConsole, &color, repeatCounter, batchStartPos, &dwBytesWritten);

                    batchStartPos.X = x + c.field.padding.x;
                    batchStartPos.Y = y + c.field.padding.y;
                }
            }
        }
    }
}
