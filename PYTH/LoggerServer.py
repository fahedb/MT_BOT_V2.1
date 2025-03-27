import asyncio
from telethon import TelegramClient
from fastapi import FastAPI, Request
import uvicorn

# הגדרות החיבור לבוט
api_id = 23246402  
api_hash = "a7b0f858ed8460c807f63ceaae0a0495"  
bot_token = "7530405359:AAFHMRHqyTVQs0kL5gsHXKf2Ah3uEumutgE"
channel_id = -1002261776362  

# יצירת לקוח Telethon **ללא הפעלת start() ישירות**
client = TelegramClient("bot_session", api_id, api_hash)

# יצירת שרת FastAPI
app = FastAPI()

@app.on_event("startup")
async def startup_event():
    """ חיבור הבוט לטלגרם לפני שהשרת מופעל """
    await client.start(bot_token=bot_token)

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

if __name__ == "__main__":
    # הפעלת השרת עם Uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8081)
