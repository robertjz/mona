/*!
  \file  KeyBoardServer
  \brief class KeyBoard information

  Copyright (c) 2004 Higepon
  All rights reserved.
  License=MIT/X Licnese

  \author  HigePon
  \version $Revision$
  \date   create:2004/01/25 update:$Date$
*/

#include <userlib.h>
#include <KeyBoardManager.h>

int regist(List<dword>* destList, MessageInfo* info);
int unregist(List<dword>* destList, MessageInfo* info);
int sendKeyInformation(KeyBoardManager* manager, List<dword>* destList, MessageInfo* info);

int MonaMain(List<char*>* pekoe)
{
    /* initilize KeyBoardManager */
    KeyBoardManager* manager = new KeyBoardManager();
    manager->init();

    /* initilize destination list */
    List<dword>* destList = new HList<dword>();
    MessageInfo info;

    /* Message loop */
    for (;;)
    {
        /* receive */
        if (!Message::receive(&info))
        {
            switch(info.header)
            {
            case MSG_KEY_SCANCODE:

                sendKeyInformation(manager, destList, &info);
                break;

            case MSG_KEY_REGIST_TO_SERVER:

                regist(destList, &info);
                break;

            case MSG_KEY_UNREGIST_FROM_SERVER:

                unregist(destList, &info);
                break;

            default:
                /* igonore this message */
                break;
            }
        }
    }
    return 0;
}

int sendKeyInformation(KeyBoardManager* manager, List<dword>* destList, MessageInfo* info)
{
    MessageInfo message;
    KeyInfo keyinfo;

    /* scan code to virtual key information */
    byte scancode = info->arg1;
    manager->setKeyScanCode(scancode);
    manager->getKeyInfo(&keyinfo);

    /* create message */
    memset(&message, 0, sizeof(MessageInfo));
    Message::create(&message, MSG_KEY_VIRTUAL_CODE, keyinfo.keycode, keyinfo.modifiers, 0, NULL);

    /* send message */
    for (int i = destList->size() - 1; i >= 0; i--)
    {
        if (Message::send(destList->get(i), &message))
        {
            printf("send error to pid = %x", destList->get(i));
            destList->removeAt(i);
        }
    }
    return 0;
}

int regist(List<dword>* destList, MessageInfo* info)
{
    dword id = info->arg1;
    destList->add(id);
    return 0;
}

int unregist(List<dword>* destList, MessageInfo* info)
{
    dword id = info->arg1;
    destList->remove(id);
    return 0;
}
