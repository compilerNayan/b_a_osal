#ifndef _USER_ACCOUNT_REPOSITORY_H_
#define _USER_ACCOUNT_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "UserAccount.h"

/* @Repository */
DefineStandardPointers(UserAccountRepository)
class UserAccountRepository : public CpaRepository<UserAccount, StdString> {
    Public Virtual ~UserAccountRepository() = default;
    
    // Custom FindBy methods
    Public Virtual optional<UserAccount> FindByName(StdString name) = 0;
};

#endif // _USER_ACCOUNT_REPOSITORY_H_

