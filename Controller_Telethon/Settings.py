import requests

SERVER_URL = "http://localhost:8080" 

HEADER_CONFIG_NAME ="ConfigName"
HEADER_CONFIG_VALUE = "ConfigValue"
HEADER_CONFIG_PERMISIION = "ConfigPermission"
HEADER_CSV_FILE_ORDERS_SUMMERY = "OrdersSummeryPath"
HEADER_STATUS_CURRENTSTATUS = "Status"

HEADER_STATUS_VALUE_STOPPED = "Stopped"
HEADER_STATUS_VALUE_RUNNING = "Running"
HEADER_STATUS_VALUE_LOADINGDATA = "LoadingData"

def Get_AllConfigsNames():
    service = "/GetAllConfigsNames"
    reqUrl = SERVER_URL + service
    session = requests.Session()
    try:

        response = session.get(reqUrl, timeout=30)  # קריאה עם זמן מקסימום של 5 שניות
        response.raise_for_status()  # אם הסטטוס אינו תקין, תזרק שגיאה

        # שמירת ה-Body לקובץ
        body_content = response.text  # שמירת ה-Body בפורמט טקסט
        lines =body_content.split("\n")
        return lines
    
    except requests.exceptions.RequestException as e:
        print(f"❌ שגיאה בביצוע הבקשה: {e}")
        return None

    
def Run_MTScanner():
    service = "/RunScanner"
    reqUrl = SERVER_URL + service
    
    try:
        response = requests.post(reqUrl, timeout=5)  # קריאה עם זמן מקסימום של 5 שניות
        response.raise_for_status()  # אם הסטטוס אינו תקין, תזרק שגיאה

        # שליפת ההדרים
        if(response.status_code == 200):
            return True
        else:
            return False
        
    except requests.exceptions.RequestException as e:
        print(f"❌ שגיאה בביצוע הבקשה: {e}")
        return None
    
def Stop_MTScanner():
    service = "/StopScanner"
    reqUrl = SERVER_URL + service
    session = requests.Session()
    try:
        response = session.post(reqUrl, timeout=5)  # קריאה עם זמן מקסימום של 5 שניות
        response.raise_for_status()  # אם הסטטוס אינו תקין, תזרק שגיאה

        # שליפת ההדרים
        if(response.status_code == 200):
            return True
        else:
            return False
        
    except requests.exceptions.RequestException as e:
        print(f"❌ שגיאה בביצוע הבקשה: {e}")
        return None
    

def Get_MTScanner_Status():
    service = "/RunningStatus"
    reqUrl = SERVER_URL + service
    session = requests.Session()

    try:
        response = session.get(reqUrl, timeout=5)  # קריאה עם זמן מקסימום של 5 שניות
        response.raise_for_status()  # אם הסטטוס אינו תקין, תזרק שגיאה

        # שליפת ההדרים
        curr_status = response.headers.get(HEADER_STATUS_CURRENTSTATUS, "Unknown")

        return {HEADER_STATUS_CURRENTSTATUS: curr_status}
    
    except requests.exceptions.RequestException as e:
        print(f"❌ שגיאה בביצוע הבקשה: {e}")
        return None
    

def Set_Config_InServer(config,value):
    service = "/SetConfigValue"
    reqUrl = SERVER_URL + service
    session = requests.Session()
    try:
        headers ={
            HEADER_CONFIG_NAME : config,
            HEADER_CONFIG_VALUE : value
        }

        response = session.post(reqUrl,headers=headers, timeout=10)  # קריאה עם זמן מקסימום של 5 שניות
        response.raise_for_status()  # אם הסטטוס אינו תקין, תזרק שגיאה
        if response.status_code == 200:
            return True
        else:
            return False

    except requests.exceptions.RequestException as e:
        print(f"❌ שגיאה בביצוע הבקשה: {e}")
        return None
    

def Get_Config_FromServer(config):
    service = "/GetConfig"
    reqUrl = SERVER_URL + service
    session = requests.Session()
    try:
        headers ={
            HEADER_CONFIG_NAME : config
        }

        response = session.get(reqUrl,headers=headers, timeout=30)  # קריאה עם זמן מקסימום של 5 שניות
        response.raise_for_status()  # אם הסטטוס אינו תקין, תזרק שגיאה

        # שליפת ההדרים
        config_name = response.headers.get(HEADER_CONFIG_NAME, "Unknown")
        config_value = response.headers.get(HEADER_CONFIG_VALUE, "Unknown")

        # שמירת ה-Body לקובץ
        body_content = response.text  # שמירת ה-Body בפורמט טקסט
        return {"ConfigName": config_name, "ConfigValue": config_value, "ConfigExplain": body_content}

    except requests.exceptions.RequestException as e:
        print(f"❌ שגיאה בביצוע הבקשה: {e}")
        return None


def Get_OrdersSummery():
    service = "/OrdersSummery_CSV"
    reqUrl = SERVER_URL + service
    session = requests.Session()
    try:
        response = session.get(reqUrl, timeout=5)  # קריאה עם זמן מקסימום של 5 שניות
        response.raise_for_status()  
        filePath = response.headers.get(HEADER_CSV_FILE_ORDERS_SUMMERY,"Unkown")
        # שליפת ההדרים
        if(response.status_code == 200):
            return filePath
        else:
            return ""
        
    except requests.exceptions.RequestException as e:
        print(f"❌ שגיאה בביצוע הבקשה: {e}")
        return None
    
