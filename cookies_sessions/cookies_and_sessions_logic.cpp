#include "./includes/cookies_and_sessions_logic.hpp"

Session& storing_session_data(SessionManager& sessionM, std::map<std::string,
    std::string>& data)
{
    Session& Ses = sessionM.createSession();
    for(std::map<std::string, std::string>::iterator it = data.begin();
        it != data.end(); ++it)
    {
        Ses.SetSessionVal(it->first, it->second);
    }
    return Ses;
}

Session& cookies_and_sessions_logic(SessionManager& sessionM, Client& client)
{
    std::string key;
    std::string CookieHeader;
    std::string SessionID;
    std::map<std::string, std::string> data;
    std::map<std::string, std::string> headers;


    headers = client.req.getHeaders();
    
    if (headers.find("COOKIE") != headers.end())
    {
        CookieHeader = headers["COOKIE"];
        data = parseCookies(CookieHeader);
        if (!data["sessionId"].empty())
            SessionID = data["sessionId"];
        else
            return storing_session_data(sessionM, data);
        if (sessionM.sessionExists(SessionID))
        {
            sessionM.sessionTimeCheck(SessionID);
            if(sessionM.sessionExists(SessionID))
            {
                Session& Ses = sessionM.getSession(SessionID);
                Ses.is_new = false;
                Ses.last_access = std::time(0);
                return Ses;
            }else
                return storing_session_data(sessionM, data);
        }
        else
            return storing_session_data(sessionM, data);
    }else
        return storing_session_data(sessionM, data);
}