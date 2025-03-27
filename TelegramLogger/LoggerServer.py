import asyncio
from telethon import TelegramClient
from fastapi import FastAPI, Request
import uvicorn

def read_bot_token(config_file="config.txt"):
    try:
        # פותחים את הקובץ לקריאה
        with open(config_file, "r") as file:
            lines = file.readlines()

        # מחפשים את השורה שבה יש את ה-BOT_TOKEN
        for line in lines:
            if line.startswith("BOT_TOKEN"):
                # פיצול השורה לפי סימן '='
                token = line.split('=')[1].strip()
                return token
        
        # אם לא מצאנו את ה-BOT_TOKEN בקובץ, נחזיר שגיאה
        raise ValueError("BOT_TOKEN לא נמצא בקובץ הקונפיגורציה.")
    
    except FileNotFoundError:
        raise FileNotFoundError(f"הקובץ {config_file} לא נמצא.")
    except Exception as e:
        raise Exception(f"בעיה בקריאת ה-BOT_TOKEN: {e}")

# הגדרות החיבור לבוט
api_id = 23246402  
api_hash = "a7b0f858ed8460c807f63ceaae0a0495"  
bot_token = read_bot_token()
channel_id = -1002261776362  
special_channel_id = -1002601966909  

# יצירת לקוח Telethon **ללא הפעלת start() ישירות**
client = TelegramClient("bot_session", api_id, api_hash)
specialClient = TelegramClient("bot_session", api_id, api_hash)

# יצירת שרת FastAPI
app = FastAPI()

@app.on_event("startup")
async def startup_event():
    """ חיבור הבוט לטלגרם לפני שהשרת מופעל """
    await client.start(bot_token=bot_token)

@app.post("/PatternsExport")
async def export_Pattern(request: Request):
    try:
        fahed = 10
        message_body = await request.body()
        message_body = message_body.decode("utf-8").strip()
        indL = request.headers.get("IndicatorLen").strip()
        barP = request.headers.get("BarPeriod").strip()

        pngs_paths = [line.strip() for line in message_body.split("\n") if line.strip()]
        
        await client.send_file(channel_id,pngs_paths[0],
        caption=(
        f"🟢 <b>Current TimeLine Pattern</b>\n\n"
        f"📏 <b>Indicator Length:</b> {indL}\n"
        f"⏱️ <b>Bar Period:</b> {barP}"
        ),parse_mode="html")
        
        pngs_paths.pop(0)
        await client.send_file(channel_id,pngs_paths,caption=(
        f"🧩 <b>Similar Patterns Found</b>\n\n"
        f"📏 <b>Indicator Length:</b> {indL}\n"
        f"⏱️ <b>Bar Period:</b> {barP}"), parse_mode="html",force_document=False,supports_streaming=True)

    except Exception as e:
        return {"status": "error", "message": str(e)}


@app.post("/LogMessage")
async def log_message(request: Request):
    """ מסלול API לקבלת הודעות ושליחתן לטלגרם """
    try:
        message_body = await request.body()
        message_body = message_body.decode("utf-8").strip()
        print("TEST:", message_body)

        if not message_body:
            return {"status": "error", "message": "Received empty log"}

        #print message body 
        await client.send_message(channel_id, f"📢📢📢📢\n {message_body}", parse_mode="html")

        return {"status": "success", "message_sent": message_body}
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.post("/SpecialLogMessage")
async def log_message(request: Request):
    """ מסלול API לקבלת הודעות ושליחתן לטלגרם """
    try:
        message_body = await request.body()
        message_body = message_body.decode("utf-8").strip()

        if not message_body:
            return {"status": "error", "message": "Received empty log"}

        #print message body 
        await client.send_message(special_channel_id, f"📢📢📢📢\n {message_body}", parse_mode="html")

        return {"status": "success", "message_sent": message_body}
    except Exception as e:
        return {"status": "error", "message": str(e)}



if __name__ == "__main__":
    # הפעלת השרת עם Uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8081)
