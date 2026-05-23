#ifndef IHTTP_CLIENT_INTERNAL_H
#define IHTTP_CLIENT_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(IHttpClient)
class IHttpClient {
    Public Virtual ~IHttpClient() = default;

    Public Virtual StdString Get(CStdString& url) = 0;

    Public Virtual StdString Post(CStdString& url,
                                CStdString& body,
                                CStdString& contentType = "application/json") = 0;

    Public Virtual StdString Put(CStdString& url,
                                CStdString& body,
                                CStdString& contentType = "application/json") = 0;

    Public Virtual StdString Delete(CStdString& url,
                                    CStdString& body = "",
                                    CStdString& contentType = "application/json") = 0;
};

#endif // IHTTP_CLIENT_INTERNAL_H