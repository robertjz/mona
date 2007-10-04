#include "GUIPlayer.h"

using namespace MonAPI;
using namespace std;

// wrapperPlayLoop for thread
static void __fastcall wrapperPlayLoop(void* p)
{
    GUIPlayer* player = (GUIPlayer*)p;
    logprintf("%s tid=%x\n", __func__, syscall_get_tid());
    player->playLoop();
}

// GUIPlayer
const string GUIPlayer::MUSIC_DIR = "/MUSIC";

GUIPlayer::GUIPlayer() : PlayFrame(NULL), command(COMMAND_NONE)
{
    logprintf("%s tid=%x\n", __func__, syscall_get_tid());
    initComponents();
    tid = mthread_create_with_arg(wrapperPlayLoop, this);
}

GUIPlayer::~GUIPlayer()
{
    destroyComponents();
    if (NULL != audio)
    {
        audio->stop();
        delete audio;
    }
    for (Songs::iterator it = songs.begin(); it != songs.end(); ++it)
    {
        delete (*it)->label;
        delete(*it);
    }
    mthread_kill(tid);
}

void GUIPlayer::initComponents()
{
    forwardImage  = new Image((MUSIC_DIR + "/FWD.JPG").data());
    backwardImage = new Image((MUSIC_DIR + "/BACK.JPG").data());

    setBounds(20, 20, 250, 375);
    forwardButton  = new ImageSinkButton(forwardImage, forwardImage, forwardImage);
    backwardButton = new ImageSinkButton(backwardImage, backwardImage, backwardImage);
    backwardButton->setBounds(0, 0, 16, 16);
    forwardButton->setBounds(20, 0, 16, 16);
    add(forwardButton);
    add(backwardButton);

}

void GUIPlayer::destroyComponents()
{
    delete forwardButton;
    delete backwardButton;
    delete forwardImage;
    delete backwardImage;
}

void GUIPlayer::processEvent(Event* e)
{
    if (e->getType() == Event::MOUSE_PRESSED && e->getSource() == forwardButton)
    {
        command = COMMAND_FORWARD;
    }
    else if (e->getType() == Event::MOUSE_PRESSED && e->getSource() == backwardButton)
    {
        command = COMMAND_BACKWARD;
    }
}

void GUIPlayer::readSongs()
{
    strings oggFiles;
    listOggfiles(MUSIC_DIR, oggFiles);
    for (int i = 0; i < oggFiles.size(); i++)
    {
        string path = oggFiles[i];
        FILE* fp = fopen(path.data(), "rb");
        if (NULL == fp)
        {
            continue;
        }
        OggVorbis_File vf;
        if (ov_open(fp, &vf, NULL, 0) < 0)
        {
            fclose(fp);
            continue;
        }
        char** infoTexts = ov_comment(&vf, -1)->user_comments;
        vorbis_info* vi=ov_info(&vf, -1);
        Song* song = new Song;
        song->path = path;
        while(*infoTexts)
        {
            if (char* p = strstr(*infoTexts, "title="))
            {
                p += strlen("title=");
                song->title = p;
            }
            else if (char* p = strstr(*infoTexts, "artist="))
            {
                p += strlen("artist=");
                song->artist = p;
            }
            ++infoTexts;
        }
        if (song->title == "")
        {
            song->title = StringHelper::basename(path.data());
        }
        if (song->artist == "")
        {
            song->artist = "Unknown Artist";
        }

        song->label = new Label((song->title + " - " + song->artist).data());
        song->label->setBounds(0, 20 * i + 20, 250, 20);
        add(song->label);
        songs.push_back(song);
        fclose(fp);
    }
    repaint();
}

void GUIPlayer::playLoop()
{
    struct audio_data_format defaultFormat = {0, 2, 16, 4400};
    audio = new Audio(&defaultFormat, AUDIO_OUTPUT);
    if (-1 == audio->start())
    {
        showError("Can not connect to AUDIO server!\n");
    }
    readSongs();
    if (songs.size() == 0)
    {
        showError("No songs found\n");
        exit(-1);
    }
            logprintf("[-1]\n");
    for (int playingIndex = 0;;)
    {
        Song* song = songs[playingIndex];
        song->label->setBackground(baygui::Color::blue);
        song->label->setForeground(baygui::Color::white);
        song->label->repaint();

        FILE* fp = fopen(song->path.data(), "rb");
        if (NULL == fp)
        {
            showError("File open error: %s.\n", song->path.data());
            continue;
        }
        OggVorbis_File vf;
            logprintf("[0]\n");
        if (ov_open(fp, &vf, NULL, 0) < 0)
        {
            showError("Skipped, %s does not appear to be an Ogg bitstream.\n", song->path.data());
            fclose(fp);
            continue;
        }
            logprintf("[0.5]\n");
        vorbis_info* vi=ov_info(&vf, -1);
        struct audio_data_format format = {0, vi->channels, 16, vi->rate};
        audio->setFormat(&format);
        int current_section;
        char pcmout[4096];
        for (;;)
        {
            logprintf("[1]\n");
            switch(command)
            {
            case COMMAND_FORWARD:
                command = COMMAND_NONE;
                goto replay;
            case COMMAND_BACKWARD:
                command = COMMAND_NONE;
                playingIndex -= 2;
                if (playingIndex < 0) playingIndex = songs.size() -1;
                goto replay;
            case COMMAND_VOLUME_UP:
            case COMMAND_VOLUME_DOWN:
                break;
            default:
                break;
            }
            logprintf("[2]\n");
            long ret = ov_read(&vf, pcmout, sizeof(pcmout), &current_section);
            logprintf("[3]\n");
            if (ret == 0)
            {
                break;
            }
            else if (ret < 0)
            {
                showError("Warning data broken?\n");
            }
            else
            {
                audio->write(pcmout, ret);
            }
        }
    replay:
            logprintf("[4]\n");
        song->label->setBackground(baygui::Color::lightGray);
        song->label->setForeground(baygui::Color::black);
        song->label->repaint();

        fclose(fp);
        playingIndex++;
        if (playingIndex >= songs.size()) playingIndex = 0;
    }
    audio->stop();
    delete audio;
    audio = NULL;
}

void GUIPlayer::paint(Graphics *g)
{
    int w = getWidth();
    int h = getHeight();
    g->setColor(baygui::Color::white);
    g->fillRect(0, 0, w, h);
}

void GUIPlayer::listOggfiles(const string& dirPath, strings& oggFiles)
{
    DIR* dir;
    if ((dir = opendir(dirPath.data())) == NULL)
    {
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        string name = entry->d_name;
        if (StringHelper::endsWith(upperCase(name), ".OGG"))
        {
            name = "/" + name;
            name = dirPath + name;
            oggFiles.push_back(name);
        }
    }

    // hack
    // we hope startup.ogg to be the first.
    reverse(oggFiles.begin(), oggFiles.end());
    closedir(dir);
}

string GUIPlayer::upperCase(const string& s)
{
    string result = s;
    transform(result.begin(), result.end(), result.begin(), toupper);
    return result;
}

void GUIPlayer::showError(const char *fmt, ...)
{
    char buf[MSG_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, MSG_BUFFER_SIZE, fmt, args);
    va_end(args);
#if 1
    _printf(buf);
#else
    statusLabel->setText(buf);
#endif
}
