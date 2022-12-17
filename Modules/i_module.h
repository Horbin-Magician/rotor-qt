#ifndef IMODEL_H
#define IMODEL_H


class IModule
{
public:
    virtual void onHotkey(unsigned int fsModifiers, unsigned int  vk);
};

#endif // IMODEL_H
