import asyncio
from telethon import TelegramClient
from fastapi import FastAPI, Request
import uvicorn

# === הגדרות ===
api_id = 23246402
api_hash = "a7b0f858ed8460c807f63ceaae0a0495"
defualt_channel_id = -1002261776362
special_channel_id = -1002601966909
tracing_channel_id = -1002697425648
error_channel_id = -1002645538128
summery_channel_id = -1002587754408

client = TelegramClient("bot_session", api_id, api_hash)
log_queue = asyncio.Queue()

ERROR_MESSAGE = 0
REGULAR_MESSAGE = 1
SPECIAL_MESSAGE = 2
TRACING_MESSAGE = 3
SUMMERY_MESSAGE = 4

app = FastAPI()

def read_bot_token(config_file="config.txt"):
    try:
        with open(config_file, "r") as file:
            lines = file.readlines()
        for line in lines:
            if line.startswith("BOT_TOKEN"):
                return line.split('=')[1].strip()
        raise ValueError("BOT_TOKEN לא נמצא בקובץ הקונפיגורציה.")
    except FileNotFoundError:
        raise FileNotFoundError(f"הקובץ {config_file} לא נמצא.")
    except Exception as e:
        raise Exception(f"בעיה בקריאת ה-BOT_TOKEN: {e}")


bot_token = read_bot_token()
@app.on_event("startup")
async def startup_event():
    await client.start(bot_token=bot_token)
    asyncio.create_task(log_worker())  # הפעלת ה-worker ברקע

# ==== API: שליחת לוג רגיל ====
@app.post("/LogMessage")
async def log_message(request: Request):
    try:
        message_body = await request.body()
        message_body = message_body.decode("utf-8").strip()
        if not message_body:
            return {"status": "error", "message": "Empty log"}
        await log_queue.put((f"📢📢📢📢\n {message_body}", REGULAR_MESSAGE))
        return {"status": "queued"}
    except Exception as e:
        return {"status": "error", "message": str(e)}

# ==== API: שליחת לוג מיוחד ====
@app.post("/SpecialLogMessage")
async def log_message_special(request: Request):
    try:
        message_body = await request.body()
        message_body = message_body.decode("utf-8").strip()
        if not message_body:
            return {"status": "error", "message": "Empty special log"}
        await log_queue.put((f"📢📢📢📢\n {message_body}", SPECIAL_MESSAGE))
        return {"status": "queued"}
    except Exception as e:
        return {"status": "error", "message": str(e)}

# ==== API: ERROR LOG ====
@app.post("/ErrMessage")
async def log_err_message(request: Request):
    try:
        message_body = await request.body()
        message_body = message_body.decode("utf-8").strip()
        if not message_body:
            return {"status": "error", "message": "Empty special log"}
        await log_queue.put((f"⚠️💥🛑❌⚠️💥🛑❌\n {message_body}", ERROR_MESSAGE))
        return {"status": "queued"}
    except Exception as e:
        return {"status": "error", "message": str(e)}

# ==== API: TRADE TRACING LOG ====
@app.post("/TraceTradeLogMessage")
async def trace_trade_message(request: Request):
    try:
        message_body = await request.body()
        message_body = message_body.decode("utf-8").strip()
        if not message_body:
            return {"status": "error", "message": "Empty special log"}
        await log_queue.put((f"🛰️🛰️🛰️🛰️🛰️\n {message_body}", TRACING_MESSAGE))
        return {"status": "queued"}
    except Exception as e:
        return {"status": "error", "message": str(e)}

# ==== API: TRADE TRACING LOG ====
@app.post("/BestDirectionSummery")
async def trace_trade_message(request: Request):
    try:
        message_body = await request.body()
        message_body = message_body.decode("utf-8").strip()
        if not message_body:
            return {"status": "error", "message": "Empty special log"}
        await log_queue.put((f"🧠🧠🧠🧠🧠\n {message_body}", SUMMERY_MESSAGE))
        return {"status": "queued"}
    except Exception as e:
        return {"status": "error", "message": str(e)}


# ==== API: שליחת תמונות לתבניות ====
@app.post("/PatternsExport")
async def export_patterns(request: Request):
    try:
        body = await request.body()
        body = body.decode("utf-8").strip()
        indL = request.headers.get("IndicatorLen", "").strip()
        barP = request.headers.get("BarPeriod", "").strip()

        pngs_paths = [line.strip() for line in body.split("\n") if line.strip()]
        if not pngs_paths:
            return {"status": "error", "message": "No PNG paths received"}

        await log_queue.put({
            "type": "patterns",
            "pngs": pngs_paths,
            "indL": indL,
            "barP": barP
        })

        return {"status": "queued"}
    except Exception as e:
        return {"status": "error", "message": str(e)}

# ==== WORKER ====
async def log_worker():
    while True:
        task = await log_queue.get()
        try:
            if isinstance(task, dict) and task.get("type") == "patterns":
                pngs = task["pngs"]
                indL = task["indL"]
                barP = task["barP"]

                if not pngs:
                    continue

                # שליחת התבנית הנוכחית
                await client.send_file(
                    defualt_channel_id,
                    pngs[0],
                    caption=(
                        f"🟢 <b>Current TimeLine Pattern</b>\n\n"
                        f"📏 <b>Indicator Length:</b> {indL}\n"
                        f"⏱️ <b>Bar Period:</b> {barP}"
                    ),
                    parse_mode="html",force_document=True
                )

                # שליחת התבניות הדומות
                if len(pngs) > 1:
                    await client.send_file(
                        channel_id,
                        pngs[1:],
                        caption=(
                            f"🧩 <b>Similar Patterns Found</b>\n\n"
                            f"📏 <b>Indicator Length:</b> {indL}\n"
                            f"⏱️ <b>Bar Period:</b> {barP}"
                        ),
                        parse_mode="html",
                        force_document=False,
                        supports_streaming=True
                    )
            else:
                # שליחת הודעת טקסט
                msg, messageType = task

                channelToSend = defualt_channel_id
                if messageType == REGULAR_MESSAGE:
                    channelToSend = defualt_channel_id
                elif messageType == ERROR_MESSAGE:
                    channelToSend = error_channel_id
                elif messageType == SPECIAL_MESSAGE:
                    channelToSend = special_channel_id
                elif messageType == TRACING_MESSAGE:
                    channelToSend = tracing_channel_id
                elif messageType == SUMMERY_MESSAGE:
                    channelToSend = summery_channel_id

                await client.send_message(
                    channelToSend,
                    msg,
                    parse_mode="html"
                )
        except Exception as e:
            print("[LogWorker] Failed to send:", e)

        log_queue.task_done()

# ===== הרצת השרת =====
if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8081)
