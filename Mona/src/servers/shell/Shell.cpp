#include <monapi/messages.h>
#include <monapi/Keys.h>

#include "Shell.h"

using namespace MonAPI;

/*----------------------------------------------------------------------
    Shell
----------------------------------------------------------------------*/
Shell::Shell(bool callAutoExec)
    : position(0), hasExited(false), callAutoExec(this->callAutoExec),
      waiting(THREAD_UNKNOWN), prevX(0), prevY(0)
{
    this->current = STARTDIR;
    this->makeApplicationList();

    if (this->callAutoExec)
    {
        this->executeMSH("/AUTOEXEC.MSH");
        this->callAutoExec = false;
    }

    this->printPrompt("\n");
    this->drawCaret();
}

Shell::~Shell()
{
}

void Shell::run()
{
    MessageInfo msg;
    while (!this->hasExited)
    {
        if (Message::receive(&msg) != 0) continue;

        switch (msg.header)
        {
            case MSG_KEY_VIRTUAL_CODE:
                if (msg.arg2 & KEY_MODIFIER_DOWN)
                {
                    this->onKeyDown(msg.arg1, msg.arg2);
                }
                break;
            case MSG_PROCESS_TERMINATED:
                if (this->waiting == msg.arg1)
                {
                    this->printPrompt("\n");
                    this->drawCaret();
                    this->waiting = THREAD_UNKNOWN;
                }
                break;
        }
    }
}

void Shell::backspace()
{
    if (this->position == 0)
    {
        /* donothing */
        return;
    }

    this->checkCaretPosition();

    monapi_call_mouse_set_cursor(0);
    int x, y;
    syscall_get_cursor(&x, &y);
    syscall_set_cursor(x - 1, y);
    printf("  ");
    syscall_set_cursor(x - 1, y);
    this->drawCaret();
    monapi_call_mouse_set_cursor(1);

    /* backspace */
    this->position--;
}

void Shell::commandChar(char c)
{
    if (c != '\0')
    {
        this->checkCaretPosition();
        int x, y;
        syscall_get_cursor(&x, &y);
        x++;
        if ((x + 1) * FONT_WIDTH >= this->screen.getWidth()) return;

        monapi_call_mouse_set_cursor(0);
        printf("%c", c);
        this->drawCaret();
        monapi_call_mouse_set_cursor(1);
    }
    this->commandLine[this->position] = c;
    this->position++;
}

void Shell::commandExecute(bool prompt)
{
    _A<CString> args = this->parseCommandLine();

    if (args.get_Length() == 0)
    {
        /* command is empty */
        monapi_call_mouse_set_cursor(0);
        this->printPrompt("\n");
        this->position = 0;
        monapi_call_mouse_set_cursor(1);
        return;
    }

    putHistory(this->commandLine);

    /* internal command */
    int isInternal = isInternalCommand(args[0]);
    if (isInternal != 0)
    {
        monapi_call_mouse_set_cursor(0);
        if (prompt) printf("\n");
        bool newline = internalCommandExecute(isInternal, args);
        if (!this->hasExited && prompt)
        {
            this->printPrompt(newline ? "\n" : NULL);
        }
        this->position = 0;
        monapi_call_mouse_set_cursor(1);
        return;
    }

    monapi_call_mouse_set_cursor(0);
    if (prompt) printf("\n");

    CString cmdLine;
    CString command = args[0].toUpper();
    if (command[0] == '/')
    {
        cmdLine = command;
    }
    else if (command.endsWith(".ELF") || command.endsWith(".EL2"))
    {
        cmdLine = APPSDIR"/" + command;
    }
    else if (command.endsWith(".APP"))
    {
        CString name = command.substring(0, command.getLength() - 4);
        cmdLine = APPSDIR"/" + name + ".APP/" + name + ".EL2";
    }
    else
    {
        for (int i = 0; i < this->apps.size(); i++)
        {
            CString file = apps.get(i);
            if (file == command + ".EL2" || file == command + ".MSH")
            {
                cmdLine = APPSDIR"/" + file;
                break;
            }
            else if (file == command + ".APP")
            {
                cmdLine = APPSDIR"/" + file + "/" + command + ".EL2";
                break;
            }
        }
        if (cmdLine == NULL) cmdLine = APPSDIR"/" + command + ".ELF";
    }

    if (cmdLine.endsWith(".MSH"))
    {
        this->executeMSH(cmdLine);
        if (this->waiting == THREAD_UNKNOWN) this->printPrompt("\n");
        monapi_call_mouse_set_cursor(1);
        return;
    }

    for (int i = 1; i < args.get_Length(); i++)
    {
        cmdLine += ' ';
        cmdLine += args[i];
    }

    dword tid;
    int result = monapi_call_elf_execute_file_get_tid(cmdLine, 1, &tid);

    this->position = 0;
    if (!this->callAutoExec && result == 0)
    {
        this->waiting = tid;
    }
    else
    {
        this->printPrompt("\n");
    }
    monapi_call_mouse_set_cursor(1);
}

void Shell::commandTerminate()
{
    commandChar('\0');
}

void Shell::putHistory(const CString& command)
{
    history.add(command);
}

CString Shell::getHistory()
{
    if (history.isEmpty()) return "";
    return history.get(0);
}

void Shell::onKeyDown(int keycode, int modifiers)
{
    if (this->waiting != THREAD_UNKNOWN)
    {
        this->printPrompt("\n");
        this->waiting = THREAD_UNKNOWN;
        if (keycode == Keys::Enter) return;
    }

    switch(keycode) {
    case(Keys::A):
    case(Keys::B):
    case(Keys::C):
    case(Keys::D):
    case(Keys::E):
    case(Keys::F):
    case(Keys::G):
    case(Keys::H):
    case(Keys::I):
    case(Keys::J):
    case(Keys::K):
    case(Keys::L):
    case(Keys::M):
    case(Keys::N):
    case(Keys::O):
    case(Keys::P):
    case(Keys::Q):
    case(Keys::R):
    case(Keys::S):
    case(Keys::T):
    case(Keys::U):
    case(Keys::V):
    case(Keys::W):
    case(Keys::X):
    case(Keys::Y):
    case(Keys::Z):
    case(Keys::Decimal):
    case(Keys::D0):
    case(Keys::D1):
    case(Keys::D2):
    case(Keys::D3):
    case(Keys::D4):
    case(Keys::D5):
    case(Keys::D6):
    case(Keys::D7):
    case(Keys::D8):
    case(Keys::D9):
    case(Keys::NumPad1):
    case(Keys::NumPad2):
    case(Keys::NumPad3):
    case(Keys::NumPad4):
    case(Keys::NumPad5):
    case(Keys::NumPad6):
    case(Keys::NumPad7):
    case(Keys::NumPad8):
    case(Keys::NumPad9):
    case(Keys::NumPad0):
    case(Keys::Subtract):
    case(Keys::Add):
    case(Keys::Space):
    case(Keys::Divide):
    case(Keys::OemPeriod):
    case(Keys::OemQuestion):
        KeyInfo key;
        key.keycode = keycode;
        key.modifiers = modifiers;
        this->commandChar(Keys::ToChar(key));
        break;
    case(Keys::Enter):
        this->drawCaret(true);
        this->commandTerminate();
        this->commandExecute(true);
        if (this->waiting == THREAD_UNKNOWN) this->drawCaret();
        break;

    case(Keys::Up):
        printf("up");
        break;
    case(Keys::Down):
        printf("down");
        break;
    case(Keys::Left):
        printf("left");
        break;
    case(Keys::Right):
        printf("right");
        break;
    case(Keys::Back):
        backspace();
        break;
    default:
        break;
    }
}

_A<CString> Shell::parseCommandLine()
{
    _A<CString> args = CString(this->commandLine).split(' ');
    int size = 0;
    FOREACH (CString, arg, args)
    {
        if (arg != NULL) size++;
    }

    _A<CString> ret(size);
    int i = 0;
    FOREACH (CString, arg, args)
    {
        if (arg == NULL) continue;

        ret[i++] = arg;
    }

    return ret;
}

int Shell::makeApplicationList()
{
    char name[15];
    int  size;

    if (syscall_cd(APPSDIR) != 0)
    {
        printf("%s: application list error\n", SVR);
        return 1;
    }

    if (syscall_dir_open() != 0)
    {
        printf("%s: application dir open error\n", SVR);
        return 2;
    }

    while (syscall_dir_read(name, &size) == 0)
    {
        CString file = name;
        if (file.endsWith(".EL2") || file.endsWith(".APP") || file.endsWith(".MSH"))
        {
            apps.add(name);
        }
    }

    syscall_dir_close();

    if (APPSDIR != this->current)
    {
        if (syscall_cd(this->current) != 0)
        {
            this->current = "/";
            syscall_cd(this->current);
        }
    }

    return 0;
}

void Shell::printPrompt(const CString& prefix /*= NULL*/)
{
    if (prefix != NULL) printf("%s", (const char*)prefix);
    printf("[Mona]%s> ", (const char*)this->current);
}

CString Shell::getParentDirectory(const CString& dir)
{
    if (dir == NULL || dir == "/") return "/";

    int p = dir.lastIndexOf('/');
    if (p < 1) return "/";

    return dir.substring(0, p);
}

CString Shell::mergeDirectory(const CString& dir1, const CString& dir2)
{
    if (dir2.startsWith("/")) return dir2.toUpper();

    CString ret = dir1;
    _A<CString> dirs = dir2.split('/');
    FOREACH (CString, d, dirs)
    {
        if (d == NULL || d == ".") continue;

        if (d == "..")
        {
            ret = this->getParentDirectory(ret);
        }
        else
        {
            if (ret != "/") ret += '/';
            ret += d.toUpper();
        }
    }
    return ret;
}

void Shell::printFiles(const CString& dir)
{
    char name[15];
    int  size;

    if (syscall_cd(dir) != 0)
    {
        printf("%s: directory not found: %s\n", SVR, (const char*)dir);
        return;
    }
    if (syscall_dir_open())
    {
        printf("%s: can not open directory: %s\n", SVR, (const char*)dir);
        return;
    }

    CString spc = "               ";
    int w = 0, sw = this->screen.getWidth();
    while (syscall_dir_read(name, &size) == 0)
    {
        CString file = name;
        if (file == "." || file == "..") continue;

        file = (file + spc).substring(0, 15);
        int fw = FONT_WIDTH * file.getLength();
        if (w + fw >= sw)
        {
            printf("\n");
            w = 0;
        }
        printf("%s", (const char*)file);
        w += fw;
    }
    printf("\n");

    syscall_dir_close();
}

void Shell::executeMSH(const CString& msh)
{
    monapi_cmemoryinfo* mi = monapi_call_file_read_data(msh, 1);
    if (mi == NULL) return;

    for (dword pos = 0, start = 0; pos <= mi->Size; pos++)
    {
        char ch = pos < mi->Size ? (char)mi->Data[pos] : '\n';
        if (ch == '\r' || ch == '\n')
        {
            int len = pos - start;
            bool prompt = true;
            if (len > 0)
            {
                if (mi->Data[start] == '@')
                {
                    start++;
                    len--;
                    prompt = false;
                }
            }
            if (len > 0)
            {
                if (len > 127) len = 127;
                strncpy(this->commandLine, (const char*)&mi->Data[start], len);
                this->position = len;
                this->commandLine[len] = '\0';
                if (prompt)
                {
                    this->printPrompt("\n");
                    printf("%s", this->commandLine);
                }
                this->commandExecute(prompt);
            }
            start = pos + 1;
        }
    }

    monapi_cmemoryinfo_dispose(mi);
    monapi_cmemoryinfo_delete(mi);
}

void Shell::drawCaret(bool erase /*= false*/)
{
    int x, y;
    syscall_get_cursor(&x, &y);
    this->screen.fillRect16(x * FONT_WIDTH, y * FONT_HEIGHT + FONT_HEIGHT - 2, 8, 2,
        erase ? BACKGROUND : FOREGROUND);
    this->prevX = x;
    this->prevY = y;
}

void Shell::checkCaretPosition()
{
    int x, y;
    syscall_get_cursor(&x, &y);
    if (this->prevX == x && this->prevY == y) return;

    monapi_call_mouse_set_cursor(0);
    this->printPrompt(this->prevX == 0 ? NULL : "\n");
    this->commandLine[this->position] = '\0';
    printf(this->commandLine);
    monapi_call_mouse_set_cursor(1);
}
