#include <SFML/Graphics.hpp>
#include <time.h>

using namespace sf;

int figures[7][4] =
{
    1,3,5,7, // I
    2,4,5,7, // Z
    3,5,4,6, // S
    3,5,4,7, // T
    2,3,5,7, // L
    3,5,7,6, // J
    2,3,4,5, // O
};

class Point {
private:
    int _x, _y;
    friend class GameWrapper;

public:
    int GetX() const { return _x; }
    int GetY() const { return _y; }
    int SetX(int x) { _x = x;}
    int SetY(int y) { _y = y;}
};

class Piece {
private:
    Point _position[4];
    Point _nextPosition[4];

    friend class GameWrapper;
public:
    Piece() {
        int n = rand() % 7;
        for (int i = 0; i < 4; i++) {
            _position[i].SetX(figures[n][i] % 2);
            _nextPosition[i].SetX(_position[i].GetX());
            
            _position[i].SetY(figures[n][i] / 2);
            _nextPosition[i].SetY(_position[i].GetY());
        }
    }
    void TryRotate() {
        Point p = _position[1];
        for (int i = 0; i < 4; i++){
            int x = _position[i].GetY() - p.GetY();
            int y = _position[i].GetX() - p.GetX();
            _nextPosition[i].SetX(p.GetX() - x);
            _nextPosition[i].SetY(p.GetY() + y);
        }
    };
    void TryMove(int dx) {
        for (int i = 0; i < 4; i++) {  
            _nextPosition[i].SetX(_position[i].GetX() + dx); 
        }
    };
    void TryMoveDown() {
        for (int i = 0; i < 4; i++) {  
            _nextPosition[i].SetX(_position[i].GetX());
            _nextPosition[i].SetY(_position[i].GetY() + 1);
        }
    }
    void MakeTransition() {
        for (int i = 0; i < 4; i++) {  
            _position[i].SetX(_nextPosition[i].GetX());
            _position[i].SetY(_nextPosition[i].GetY());
        }
    }
};

class GameBoard {
private:
    int _width, _height;
    int** _field;

    friend class GameWrapper;

public:
    GameBoard(int width = 10, int height = 20) {
        _width = width;
        _height = height;
        
        _field = new int*[height];
        for (int i = 0; i < height; i++) {
            _field[i] = new int[width];

            for (int j = 0; j < width; j++) {
                _field[i][j] = 0;
            }
        }
    }
    ~GameBoard() {
        for (int i = 0; i < _height; i++) {
            delete[] _field[i];
        }
        delete[] _field;
    }
    void CheckWin() {
        int k = _height - 1;
        for (int i = _height - 1; i >= 0; i--) {
            int count = 0;
            for (int j = 0; j < _width; j++) {
                if (_field[i][j]) count += 1;
                _field[k][j] = _field[i][j];
            }
            if (count < _width) k -= 1;
        }
    };
};

class GameWrapper {
private:
    GameBoard* _gameBoard;
    Piece *_piece;
    bool _isLost;

public:
    GameWrapper(int width = 10, int height = 20) {
        _gameBoard = new GameBoard(width, height);
        _piece = new Piece();
        _isLost = false;
    }
    ~GameWrapper() {
        delete _gameBoard;
        delete _piece;
    }
    bool CheckValid(Piece *piece) {
        for (int i = 0; i < 4; i++) {
            
            if (
                piece->_nextPosition[i]._x < 0 
                || piece->_nextPosition[i]._x >= _gameBoard->_width 
                || piece->_nextPosition[i]._y >= _gameBoard->_height 
                ) {
                    
                    return false;
                }
            else if (_gameBoard->_field[piece->_nextPosition[i]._y][piece->_nextPosition[i]._x]) {
                return false;
            }
        }
        return true;
    };
    void UpdateState() {
        _gameBoard->CheckWin();
        _piece->TryMoveDown();
        if (CheckValid(_piece)) _piece->MakeTransition();
        else {
            for (int i = 0; i < 4; i++) {
                _gameBoard->_field[_piece->_position[i]._y][_piece->_position[i]._x] = 1;
            }
            TryInstantiate();
        }
    }
    void TryInstantiate() {
        Piece *delPiece = _piece;
        Piece *newPiece = new Piece();
        _piece = newPiece;
        delete delPiece;

        if (!CheckValid(_piece)) {
            delete _piece;
            _isLost = true;
        }
    }
    void Move(int dx) {
        _piece->TryMove(dx);
        if (CheckValid(_piece)) _piece->MakeTransition();
    }
    void Rotate() {
        _piece->TryRotate();
        if (CheckValid(_piece)) _piece->MakeTransition();
    }
    bool CheckLost() {
        return _isLost;
    }
    int GetWidth() {
        return _gameBoard->_width;
    }
    int GetHeight() {
        return _gameBoard->_height;
    }
    int** GetField() {
        return _gameBoard->_field;
    }
    Point* GetPiecePosition() {
        return _piece->_position;
    }
};

class GameView {
private:
    GameWrapper *_gameManager;
    
public:
    GameView(int width = 10, int height = 20) {
        _gameManager = new GameWrapper(width, height);
    }
    ~GameView() {
        delete _gameManager;
    }
    void RunGame() {
        GameWrapper game;

        RenderWindow window(VideoMode(320, 480), "Tetris");

        Texture t1, t2;
        t1.loadFromFile("images/tiles.png");
        t2.loadFromFile("images/frame.png");
        Sprite s(t1);
        Sprite frame(t2);

        Clock clock;
        float timer = 0, delay = 0.3;
        
        while (window.isOpen()) {
            if (game.CheckLost()) {
                window.close();
            }
            float time = clock.getElapsedTime().asSeconds();
            clock.restart();
            timer += time;

            Event e;

            if (Keyboard::isKeyPressed(Keyboard::Down)) delay = 0.05;

            //Similar to FixedUpdate (But not the same)
            if (timer > delay) {
                game.UpdateState();
                timer = 0;
            }
            delay = 0.3;

            while (window.pollEvent(e)) {
                if (e.type == Event::Closed)
                    window.close();
                
                if (e.type == Event::KeyPressed) {
                    switch (e.key.code) {
                        case Keyboard::Up:
                            game.Rotate();
                            break;
                        
                        case Keyboard::Left:
                            game.Move(-1);
                            break;
                        
                        case Keyboard::Right:
                            game.Move(1);
                            break;
                        
                        default:
                            break;
                    }
                }
            }

            //Draw
            window.clear(Color::White);    
            
            for (int i = 0; i < game.GetHeight(); i++) {
                for (int j = 0; j < game.GetWidth(); j++) {
                    if (game.GetField()[i][j] == 0) continue;
                    s.setTextureRect(IntRect(game.GetField()[i][j] * 18, 0, 18, 18));
                    s.setPosition(j * 18,i * 18);
                    s.move(28,31);
                    window.draw(s);
                }
            }

            for (int i = 0; i < 4; i++) {
                s.setTextureRect(IntRect(1 * 18, 0, 18, 18));
                s.setPosition(game.GetPiecePosition()[i].GetX()*18, game.GetPiecePosition()[i].GetY()*18);
                s.move(28,31);
                window.draw(s);
            }
            window.draw(frame);
            window.display();
            
        }
    }
};

int main() {
    GameView* game = new GameView();
    game->RunGame();
    delete game;
    
}