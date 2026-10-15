#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif


#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <utility>

// OOP Mini Project: OOP Quiz App
// Native Windows GUI using the Win32 API. No external libraries are required.

using std::string;
using std::wstring;
using std::vector;
using std::unique_ptr;
using std::make_unique;

// ----------------------------- CO1: OOP -----------------------------
// Encapsulation: question data is stored inside this base class.
// CO2: derived question types reuse common Question functionality.
// CO3: virtual functions provide runtime polymorphism.
class Question {
protected:
    wstring prompt_;
    vector<wstring> options_;
    int answerIndex_;
    wstring explanation_;

public:
    Question(wstring prompt, vector<wstring> options, int answerIndex, wstring explanation)
        : prompt_(std::move(prompt)), options_(std::move(options)),
          answerIndex_(answerIndex), explanation_(std::move(explanation)) {}
    virtual ~Question() = default;

    const wstring& prompt() const { return prompt_; }
    const vector<wstring>& options() const { return options_; }
    const wstring& explanation() const { return explanation_; }
    virtual wstring typeName() const = 0;
    virtual bool isCorrect(int selectedIndex) const { return selectedIndex == answerIndex_; }
    int correctIndex() const { return answerIndex_; }
};

class MultipleChoiceQuestion final : public Question {
public:
    using Question::Question;
    wstring typeName() const override { return L"Multiple choice"; }
};

class TrueFalseQuestion final : public Question {
public:
    using Question::Question;
    wstring typeName() const override { return L"True / False"; }
};

// CO5: STL vector + smart pointers provide reusable, safe storage for base-class objects.
class QuestionBank {
    vector<unique_ptr<Question>> questions_;
public:
    void add(unique_ptr<Question> question) { questions_.push_back(std::move(question)); }
    size_t size() const { return questions_.size(); }
    Question& at(size_t index) { return *questions_.at(index); }
    const Question& at(size_t index) const { return *questions_.at(index); }
    void shuffle() {
        static std::mt19937 generator{ std::random_device{}() };
        std::shuffle(questions_.begin(), questions_.end(), generator);
    }
    bool empty() const { return questions_.empty(); }
};

// CO5: reusable class template; the same scoring logic can work with different numeric types.
template <typename T>
class ScoreKeeper {
    T score_{};
    size_t answered_ = 0;
public:
    void record(bool correct) {
        ++answered_;
        if (correct) score_ += static_cast<T>(1);
    }
    T score() const { return score_; }
    size_t answered() const { return answered_; }
    void reset() { score_ = T{}; answered_ = 0; }
};

// Generic template function (CO5).
template <typename T>
T clampValue(T value, T low, T high) {
    return value < low ? low : (value > high ? high : value);
}

static wstring utf8ToWide(const string& text) {
    if (text.empty()) return L"";
    int count = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (count <= 0) return wstring(text.begin(), text.end());
    vector<wchar_t> buffer(static_cast<size_t>(count));
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buffer.data(), count);
    return wstring(buffer.data());
}

static vector<string> splitPipe(const string& line) {
    vector<string> fields;
    std::stringstream stream(line);
    string item;
    while (std::getline(stream, item, '|')) fields.push_back(item);
    // Preserve a final empty field if the line ends with '|'.
    if (!line.empty() && line.back() == '|') fields.emplace_back();
    return fields;
}

// CO4: file handling. Each valid line has this shape:
// MCQ|prompt|option1|option2|option3|option4|correctIndex(0-based)|explanation
// TF|prompt|True|False|||correctIndex(0-based)|explanation
static size_t loadQuestionsFromFile(const string& path, QuestionBank& bank) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Could not open questions.txt");

    string line;
    size_t loaded = 0;
    size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (line.empty() || line[0] == '#') continue;
        try {
            auto fields = splitPipe(line);
            if (fields.size() != 8) throw std::runtime_error("Expected 8 fields");
            int answer = std::stoi(fields[6]);
            string kind = fields[0];
            vector<wstring> options;
            if (kind == "TF") {
                options = { utf8ToWide(fields[2]), utf8ToWide(fields[3]) };
                if (answer < 0 || answer > 1) throw std::runtime_error("TF answer must be 0 or 1");
                bank.add(make_unique<TrueFalseQuestion>(utf8ToWide(fields[1]), options, answer,
                                                       utf8ToWide(fields[7])));
            } else if (kind == "MCQ") {
                options = { utf8ToWide(fields[2]), utf8ToWide(fields[3]),
                            utf8ToWide(fields[4]), utf8ToWide(fields[5]) };
                if (answer < 0 || answer > 3) throw std::runtime_error("MCQ answer must be 0 to 3");
                bank.add(make_unique<MultipleChoiceQuestion>(utf8ToWide(fields[1]), options, answer,
                                                             utf8ToWide(fields[7])));
            } else {
                throw std::runtime_error("Type must be MCQ or TF");
            }
            ++loaded;
        } catch (const std::exception&) {
            // Ignore malformed lines so one typo does not stop the whole quiz.
        }
    }
    if (loaded == 0) throw std::runtime_error("No valid questions found in questions.txt");
    return loaded;
}

static void addBuiltInQuestions(QuestionBank& bank) {
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which OOP concept means one interface can have multiple forms?",
        vector<wstring>{L"Encapsulation", L"Polymorphism", L"Compilation", L"Tokenization"}, 1,
        L"Polymorphism allows the same interface to behave differently for different object types."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which access specifier hides class data from direct outside access?",
        vector<wstring>{L"public", L"private", L"global", L"virtual"}, 1,
        L"Private members can be accessed directly only from inside the class and its friends."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which feature allows a derived class to reuse a base class?",
        vector<wstring>{L"Inheritance", L"Casting", L"File handling", L"Exception handling"}, 0,
        L"Inheritance lets a derived class reuse and extend the accessible behaviour of a base class."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"What makes a base-class function support runtime polymorphism in C++?",
        vector<wstring>{L"static", L"friend", L"virtual", L"mutable"}, 2,
        L"A virtual function can dispatch to the derived implementation through a base pointer or reference."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which symbol is used to declare a pure virtual function?",
        vector<wstring>{L"== 0", L"= 0", L"!= 0", L"=> 0"}, 1,
        L"A pure virtual function is declared with = 0, making its class abstract unless implemented appropriately."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which C++ feature lets a function name be reused with different parameter lists?",
        vector<wstring>{L"Function overloading", L"Function hiding only", L"File opening", L"Type erasure"}, 0,
        L"Function overloading provides multiple functions with the same name and different parameter lists."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which keyword can prevent an implicit conversion through a single-argument constructor?",
        vector<wstring>{L"mutable", L"explicit", L"virtual", L"typename"}, 1,
        L"The explicit keyword prevents certain implicit conversions through constructors and conversion functions."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which STL container is commonly used to store a resizable sequence of questions?",
        vector<wstring>{L"std::vector", L"std::exception", L"std::ifstream", L"std::string_view only"}, 0,
        L"std::vector is a dynamic sequence container that grows as elements are added."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which stream class is commonly used to read a text file?",
        vector<wstring>{L"std::ofstream", L"std::ifstream", L"std::ostream only", L"std::thread"}, 1,
        L"std::ifstream is an input file stream used to read data from files."));
    bank.add(make_unique<MultipleChoiceQuestion>(L"Which C++ block handles an exception that was thrown?",
        vector<wstring>{L"try", L"throw", L"catch", L"finally"}, 2,
        L"A catch block handles an exception thrown from a corresponding try block. C++ does not have a standard finally block."));
    bank.add(make_unique<TrueFalseQuestion>(L"A class with a pure virtual function is an abstract class.",
        vector<wstring>{L"True", L"False"}, 0,
        L"A class with at least one pure virtual function is abstract and cannot be instantiated directly."));
    bank.add(make_unique<TrueFalseQuestion>(L"A virtual destructor is useful when deleting a derived object through a base-class pointer.",
        vector<wstring>{L"True", L"False"}, 0,
        L"A virtual base destructor ensures the derived destructor is also invoked during deletion through a base pointer."));
}

struct Attempt {
    int selected = -1;
    bool correct = false;
};

constexpr int ID_ACTION = 1001;
constexpr int ID_RESTART = 1002;
constexpr int ID_CLOSE = 1003;
constexpr int ID_OPTION_BASE = 1100;
constexpr int ID_TITLE = 1200;
constexpr int ID_SUBTITLE = 1201;
constexpr int ID_PROGRESS = 1202;
constexpr int ID_GROUP = 1203;
constexpr int ID_QUESTION = 1204;
constexpr int ID_FEEDBACK = 1205;
constexpr int ID_FOOTER = 1206;
constexpr int ID_RESULT = 1207;

struct AppState {
    QuestionBank bank;
    ScoreKeeper<int> score;
    vector<Attempt> attempts;
    size_t current = 0;
    int selected = -1;
    bool answeredCurrent = false;
    bool finished = false;
    bool usingBuiltIn = false;
    string loadMessage = "";

    HWND title = nullptr, subtitle = nullptr, progress = nullptr, group = nullptr;
    HWND questionText = nullptr, feedback = nullptr, footer = nullptr, resultText = nullptr;
    HWND options[4] = { nullptr, nullptr, nullptr, nullptr };
    HWND actionButton = nullptr, restartButton = nullptr, closeButton = nullptr;
    HFONT titleFont = nullptr, subtitleFont = nullptr, questionFont = nullptr, bodyFont = nullptr;
    HBRUSH backgroundBrush = nullptr;
};

static AppState g;
static HWND gMainWindow = nullptr;

static void setFont(HWND control, HFONT font) {
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

static HWND createControl(HWND parent, const wchar_t* className, const wchar_t* text,
                          DWORD style, int x, int y, int width, int height, int id) {
    return CreateWindowExW(0, className, text, WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandleW(nullptr), nullptr);
}

static void setOptionsEnabled(bool enabled) {
    for (HWND option : g.options) if (option) EnableWindow(option, enabled ? TRUE : FALSE);
}

static void clearOptionChecks() {
    for (HWND option : g.options) if (option) SendMessageW(option, BM_SETCHECK, BST_UNCHECKED, 0);
}

static wstring optionLabel(size_t questionIndex, int optionIndex) {
    if (questionIndex >= g.bank.size()) return L"";
    const auto& q = g.bank.at(questionIndex);
    if (optionIndex < 0 || static_cast<size_t>(optionIndex) >= q.options().size()) return L"";
    return q.options()[static_cast<size_t>(optionIndex)];
}

static void renderQuestion() {
    if (g.bank.empty() || g.current >= g.bank.size()) return;
    const Question& q = g.bank.at(g.current); // base reference uses virtual functions (runtime polymorphism)
    wstring progressText = L"QUESTION " + std::to_wstring(g.current + 1) + L" OF " +
        std::to_wstring(g.bank.size()) + L"     |     " + q.typeName() + L"     |     SCORE: " + std::to_wstring(g.score.score());
    SetWindowTextW(g.progress, progressText.c_str());

    wstring prompt = q.prompt();
    prompt = L"Q" + std::to_wstring(g.current + 1) + L".  " + prompt;
    SetWindowTextW(g.questionText, prompt.c_str());

    for (int i = 0; i < 4; ++i) {
        if (static_cast<size_t>(i) < q.options().size()) {
            wstring label = wchar_t('A' + i) + wstring(L".  ") + q.options()[static_cast<size_t>(i)];
            SetWindowTextW(g.options[i], label.c_str());
            ShowWindow(g.options[i], SW_SHOW);
            EnableWindow(g.options[i], TRUE);
        } else {
            SetWindowTextW(g.options[i], L"");
            ShowWindow(g.options[i], SW_HIDE);
        }
    }
    clearOptionChecks();
    g.selected = -1;
    g.answeredCurrent = false;
    SetWindowTextW(g.feedback, L"Choose one answer, then click Check Answer.");
    SetWindowTextW(g.actionButton, L"Check Answer  >");
}

static void saveResultToFile() {
    // CO4: append quiz results using file handling and handle errors at the call site.
    std::ofstream out("results.txt", std::ios::app);
    if (!out) throw std::runtime_error("Could not open results.txt for writing");
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm localTime{};
#ifdef _MSC_VER
    localtime_s(&localTime, &now);
#else
    if (const std::tm* timeInfo = std::localtime(&now)) localTime = *timeInfo;
#endif
    out << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
        << " | Score: " << g.score.score() << "/" << g.bank.size()
        << " | Percentage: " << (g.bank.size() ? (100 * g.score.score() / static_cast<int>(g.bank.size())) : 0)
        << "%\n";
    if (!out) throw std::runtime_error("Failed while writing results.txt");
}

static void finishQuiz() {
    g.finished = true;
    ShowWindow(g.group, SW_HIDE);
    ShowWindow(g.progress, SW_HIDE);
    ShowWindow(g.questionText, SW_HIDE);
    ShowWindow(g.feedback, SW_HIDE);
    for (HWND option : g.options) ShowWindow(option, SW_HIDE);
    ShowWindow(g.actionButton, SW_HIDE);

    const int total = static_cast<int>(g.bank.size());
    const int correct = g.score.score();
    const int percentage = total ? (100 * correct / total) : 0;
    wstring report = L"QUIZ COMPLETE!\r\n\r\nYour score: " + std::to_wstring(correct) + L" / " +
        std::to_wstring(total) + L"  (" + std::to_wstring(percentage) + L"%)\r\n\r\n";

    int incorrectCount = 0;
    for (size_t i = 0; i < g.attempts.size(); ++i) {
        if (!g.attempts[i].correct) ++incorrectCount;
    }
    report += L"Incorrect answers: " + std::to_wstring(incorrectCount) + L"\r\n";
    if (incorrectCount > 0) {
        report += L"Review:\r\n";
        int shown = 0;
        for (size_t i = 0; i < g.attempts.size() && shown < 7; ++i) {
            if (g.attempts[i].correct) continue;
            const Question& q = g.bank.at(i);
            report += L"\r\n" + std::to_wstring(i + 1) + L". " + q.prompt() + L"\r\n   Correct answer: " +
                optionLabel(i, q.correctIndex()) + L"\r\n";
            ++shown;
        }
        if (incorrectCount > shown) report += L"\r\n...open the question file to review all concepts.";
    } else {
        report += L"Excellent! You answered every question correctly.";
    }
    report += L"\r\n\r\nResults are appended to results.txt when saving succeeds.";
    SetWindowTextW(g.resultText, report.c_str());
    ShowWindow(g.resultText, SW_SHOW);
    ShowWindow(g.restartButton, SW_SHOW);
    ShowWindow(g.closeButton, SW_SHOW);

    try {
        saveResultToFile();
    } catch (const std::exception&) {
        SetWindowTextW(g.footer, L"Quiz completed, but results.txt could not be saved. Check folder permissions.");
    }
}

static void restartQuiz() {
    g.score.reset();
    g.attempts.clear();
    g.current = 0;
    g.selected = -1;
    g.answeredCurrent = false;
    g.finished = false;
    g.bank.shuffle();
    ShowWindow(g.resultText, SW_HIDE);
    ShowWindow(g.restartButton, SW_HIDE);
    ShowWindow(g.closeButton, SW_HIDE);
    ShowWindow(g.group, SW_SHOW);
    ShowWindow(g.progress, SW_SHOW);
    ShowWindow(g.questionText, SW_SHOW);
    ShowWindow(g.feedback, SW_SHOW);
    ShowWindow(g.actionButton, SW_SHOW);
    renderQuestion();
}

static void onAction() {
    if (g.finished) return;
    if (!g.answeredCurrent) {
        if (g.selected < 0) {
            MessageBoxW(gMainWindow, L"Please select an answer first.", L"No answer selected", MB_OK | MB_ICONINFORMATION);
            return;
        }
        const Question& q = g.bank.at(g.current);
        const bool correct = q.isCorrect(g.selected);
        g.score.record(correct);
        g.attempts.push_back({ g.selected, correct });
        g.answeredCurrent = true;
        setOptionsEnabled(false);

        wstring feedbackText = correct ? L"Correct!  " : L"Not quite.  Correct answer: ";
        if (!correct) feedbackText += optionLabel(g.current, q.correctIndex()) + L".  ";
        feedbackText += q.explanation();
        SetWindowTextW(g.feedback, feedbackText.c_str());
        wstring progressText = L"QUESTION " + std::to_wstring(g.current + 1) + L" OF " +
            std::to_wstring(g.bank.size()) + L"     |     " + q.typeName() + L"     |     SCORE: " + std::to_wstring(g.score.score());
        SetWindowTextW(g.progress, progressText.c_str());
        SetWindowTextW(g.actionButton, (g.current + 1 == g.bank.size()) ? L"View Results  >" : L"Next Question  >");
    } else {
        if (g.current + 1 >= g.bank.size()) {
            finishQuiz();
            return;
        }
        // Generic template helper used to keep an index inside valid bounds (CO5).
        g.current = clampValue<size_t>(g.current + 1, static_cast<size_t>(0), g.bank.size() - 1);
        renderQuestion();
    }
}

static void createFonts() {
    g.titleFont = CreateFontW(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g.subtitleFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g.questionFont = CreateFontW(20, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g.bodyFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

static void makeGui(HWND hwnd) {
    g.backgroundBrush = CreateSolidBrush(RGB(245, 248, 253));
    createFonts();

    g.title = createControl(hwnd, L"STATIC", L"OOP QUIZ STUDIO", SS_LEFT, 36, 22, 800, 42, ID_TITLE);
    g.subtitle = createControl(hwnd, L"STATIC", L"C++ Object-Oriented Programming  |   Mini Project", SS_LEFT, 39, 67, 800, 27, ID_SUBTITLE);
    g.progress = createControl(hwnd, L"STATIC", L"QUESTION 1 OF 1  |  SCORE: 0", SS_LEFT, 40, 112, 800, 26, ID_PROGRESS);
    g.group = createControl(hwnd, L"BUTTON", L"  YOUR QUESTION", BS_GROUPBOX, 30, 145, 815, 390, ID_GROUP);
    g.questionText = createControl(hwnd, L"STATIC", L"Loading question...", SS_LEFT | SS_NOPREFIX, 54, 178, 760, 82, ID_QUESTION);

    g.options[0] = createControl(hwnd, L"BUTTON", L"A. Option 1", BS_AUTORADIOBUTTON | WS_GROUP | BS_MULTILINE, 62, 274, 750, 42, ID_OPTION_BASE);
    g.options[1] = createControl(hwnd, L"BUTTON", L"B. Option 2", BS_AUTORADIOBUTTON | BS_MULTILINE, 62, 324, 750, 42, ID_OPTION_BASE + 1);
    g.options[2] = createControl(hwnd, L"BUTTON", L"C. Option 3", BS_AUTORADIOBUTTON | BS_MULTILINE, 62, 374, 750, 42, ID_OPTION_BASE + 2);
    g.options[3] = createControl(hwnd, L"BUTTON", L"D. Option 4", BS_AUTORADIOBUTTON | BS_MULTILINE, 62, 424, 750, 42, ID_OPTION_BASE + 3);
    g.feedback = createControl(hwnd, L"STATIC", L"Choose one answer, then click Check Answer.", SS_LEFT | SS_NOPREFIX, 46, 548, 775, 54, ID_FEEDBACK);
    g.actionButton = createControl(hwnd, L"BUTTON", L"Check Answer  >", BS_PUSHBUTTON, 633, 608, 190, 44, ID_ACTION);
    g.footer = createControl(hwnd, L"STATIC", L"Question bank loading...", SS_LEFT, 40, 663, 770, 24, ID_FOOTER);
    g.resultText = createControl(hwnd, L"STATIC", L"", SS_LEFT | SS_NOPREFIX, 54, 166, 775, 420, ID_RESULT);
    g.restartButton = createControl(hwnd, L"BUTTON", L"Try Again", BS_PUSHBUTTON, 580, 608, 135, 44, ID_RESTART);
    g.closeButton = createControl(hwnd, L"BUTTON", L"Close", BS_PUSHBUTTON, 730, 608, 95, 44, ID_CLOSE);

    setFont(g.title, g.titleFont);
    setFont(g.subtitle, g.subtitleFont);
    setFont(g.progress, g.bodyFont);
    setFont(g.group, g.bodyFont);
    setFont(g.questionText, g.questionFont);
    for (HWND option : g.options) setFont(option, g.bodyFont);
    setFont(g.feedback, g.bodyFont);
    setFont(g.footer, g.subtitleFont);
    setFont(g.resultText, g.bodyFont);
    setFont(g.actionButton, g.bodyFont);
    setFont(g.restartButton, g.bodyFont);
    setFont(g.closeButton, g.bodyFont);

    ShowWindow(g.resultText, SW_HIDE);
    ShowWindow(g.restartButton, SW_HIDE);
    ShowWindow(g.closeButton, SW_HIDE);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        gMainWindow = hwnd;
        makeGui(hwnd);
        try {
            loadQuestionsFromFile("questions.txt", g.bank);
            g.loadMessage = "Loaded questions.txt";
        } catch (const std::exception&) {
            addBuiltInQuestions(g.bank);
            g.usingBuiltIn = true;
            g.loadMessage = "Using built-in questions (questions.txt was not found or had no valid questions).";
        }
        g.bank.shuffle();
        SetWindowTextW(g.footer, utf8ToWide(g.loadMessage).c_str());
        renderQuestion();
        return 0;

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id >= ID_OPTION_BASE && id < ID_OPTION_BASE + 4 && HIWORD(wParam) == BN_CLICKED) {
            if (!g.answeredCurrent && !g.finished) g.selected = id - ID_OPTION_BASE;
            return 0;
        }
        if (id == ID_ACTION && HIWORD(wParam) == BN_CLICKED) { onAction(); return 0; }
        if (id == ID_RESTART && HIWORD(wParam) == BN_CLICKED) { restartQuiz(); return 0; }
        if (id == ID_CLOSE && HIWORD(wParam) == BN_CLICKED) { DestroyWindow(hwnd); return 0; }
        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        HWND control = reinterpret_cast<HWND>(lParam);
        SetBkMode(hdc, TRANSPARENT);
        if (control == g.title) SetTextColor(hdc, RGB(25, 53, 86));
        else if (control == g.subtitle || control == g.footer) SetTextColor(hdc, RGB(83, 105, 131));
        else if (control == g.progress) SetTextColor(hdc, RGB(27, 92, 145));
        else if (control == g.feedback) SetTextColor(hdc, RGB(36, 76, 104));
        return reinterpret_cast<LRESULT>(g.backgroundBrush);
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetBkMode(hdc, TRANSPARENT);
        return reinterpret_cast<LRESULT>(g.backgroundBrush);
    }
    case WM_ERASEBKGND: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(reinterpret_cast<HDC>(wParam), &rect, g.backgroundBrush);
        return 1;
    }
    case WM_DESTROY:
        if (g.titleFont) DeleteObject(g.titleFont);
        if (g.subtitleFont) DeleteObject(g.subtitleFont);
        if (g.questionFont) DeleteObject(g.questionFont);
        if (g.bodyFont) DeleteObject(g.bodyFont);
        if (g.backgroundBrush) DeleteObject(g.backgroundBrush);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"OOPQuizStudioWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&wc)) {
        MessageBoxW(nullptr, L"Could not register the application window.", L"OOP Quiz Studio", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"OOP Quiz Studio - C++ Mini Project",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 750,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd) {
        MessageBoxW(nullptr, L"Could not create the application window.", L"OOP Quiz Studio", MB_OK | MB_ICONERROR);
        return 1;
    }
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
