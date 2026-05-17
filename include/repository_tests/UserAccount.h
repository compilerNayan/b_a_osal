#ifndef USERACCOUNT_H
#define USERACCOUNT_H

#include <StandardDefines.h>

/* @Entity */
class UserAccount {

    /* @Id */
    /* @NotNull */
    Public optional<StdString> username;

    Public optional<StdString> password;

    Public optional<StdString> name;
};

#endif

