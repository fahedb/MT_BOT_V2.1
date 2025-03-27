
import asyncio
from telethon import TelegramClient, events, Button
from Settings import (
    Get_Config_FromServer,
    Get_MTScanner_Status,
    Run_MTScanner,
    Stop_MTScanner,
    Get_AllConfigsNames,
    Set_Config_InServer,
    HEADER_CONFIG_NAME,
    HEADER_CONFIG_VALUE,
    HEADER_CONFIG_PERMISIION,
    HEADER_STATUS_CURRENTSTATUS,
    HEADER_STATUS_VALUE_STOPPED,
    HEADER_STATUS_VALUE_RUNNING,
    HEADER_STATUS_VALUE_LOADINGDATA,
)

# 🔑 סיסמה למשתמשים שרוצים גישה
ADMIN_PASSWORD = "TempPassword"
STATUS_SETTINGS = "settings"
STATUS_MAIN_MENU = "main_menu"


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


# יצירת לקוח טלגרם כ-Bot API
api_id = 23246402  
api_hash = "a7b0f858ed8460c807f63ceaae0a0495"  
bot_token = read_bot_token()
channel_id = -1002662838894
client = TelegramClient("bot_session", api_id, api_hash).start(bot_token=bot_token)

# 📌 רשימת משתמשים שאושרו לאחר שהזינו סיסמה
authorized_users = {}
user_states = {}  # מילון לניהול הסטטוס של המשתמשים
print(bot_token);
All_Configs_names = Get_AllConfigsNames()  # שליפת כל שמות ההגדרות
print("here2")



# 📌 פונקציה לאימות משתמשים עם סיסמה
async def authorize_user(event):
    user_id = event.sender_id
    message_text = event.message.text.strip()

    if message_text == ADMIN_PASSWORD:
        authorized_users[user_id] = True
        await show_main_menu(event)
    else:
        await event.respond("❌ סיסמה שגויה! נסה שוב.")
        await event.respond("יש להכניס את הסיסמה שלך כדי לשלוט בבוט:")

# 📌 פונקציה לבדוק אם המשתמש מורשה
def is_authorized(user_id):
    return authorized_users.get(user_id, False)

# 📌 תפריט ראשי עם כפתורים
async def show_main_menu(event):
    buttons = [
        [Button.inline("▶️ הרצת הבוט", b"run_bot"), Button.inline("⏹ עצירת הבוט", b"stop_bot")],
        [Button.inline("📊 סטטוס הבוט", b"status_bot")],
        [Button.inline("⚙️ הגדרות", b"settings")],
    ]
    await event.respond("🎛 **תפריט ראשי**\nבחר פעולה:", buttons=buttons)

# 📌 תפריט ההגדרות עם כפתורים
async def settings_bot(event):
    buttons = [
        [Button.inline("⚙️ שינוי הגדרה", b"set_settings")],
        [Button.inline("📊 הצגת ערך הגדרה", b"get_settings")],
        [Button.inline("🔙 חזרה", b"main_menu")],
    ]
    await event.respond("⚙️ **תפריט ההגדרות**\nבחר פעולה:", buttons=buttons)

# 📌 מאזין לכל הודעה ומחזיר תפריט כפתורים
@client.on(events.NewMessage)
async def handle_message(event):
    user_id = event.sender_id
    
    if not is_authorized(user_id):
        await authorize_user(event)
    elif user_states.get(user_id) is None:
        await show_main_menu(event)
    elif user_id in user_states and user_states[user_id].startswith("awaiting_config_value_"):
        config_name = user_states[user_id].replace("awaiting_config_value_", "")
        message_text = event.message.text.strip()
        print(f"🔄 [handle_user_input] המשתמש {user_id} שולח ערך חדש עבור {config_name}: {message_text}")

        user_states[user_id] = None  # מסירים את הסטטוס מהמשתמש

        await event.respond(f"🔄 מעדכן את `{config_name}` לערך `{message_text}`...")

        # קריאה לפונקציה ששומרת את ההגדרה החדשה
        success = Set_Config_InServer(config_name, message_text)
        senderDetails = await event.get_sender()
        user_Name = senderDetails.username if senderDetails.username else user_id
        await client.send_message(channel_id,
        f"<b>🔧 הגדרה עודכנה!</b>\n"
        f"👤 <b>משתמש:</b> {user_Name}\n"
        f"⚙️ <b>הגדרה:</b> {config_name}\n"
        f"🆕 <b>ערך חדש:</b> {message_text}",parse_mode="html")

        if success:
            await event.respond(f"✅ **ההגדרה `{config_name}` עודכנה בהצלחה!**")
        else:
            await event.respond(f"❌ **עדכון ההגדרה `{config_name}` נכשל.**")
        

# 📌 מאזין להודעות ומציג את התפריט
@client.on(events.NewMessage(pattern="/start"))
async def start(event):
    user_id = event.sender_id
    if not is_authorized(user_id):
        await event.respond("🔑 **יש להזין סיסמה כדי לקבל גישה:**")
    elif user_states.get(event.sender_id) is None:
        await show_main_menu(event)
    elif user_id in user_states and user_states[user_id].startswith("awaiting_config_value_"):
        config_name = user_states[user_id].replace("awaiting_config_value_", "")
        message_text = event.message.text.strip()
        print(f"🔄 [handle_user_input] המשתמש {user_id} שולח ערך חדש עבור {config_name}: {message_text}")

        user_states[user_id] = None  # מסירים את הסטטוס מהמשתמש

        await event.respond(f"🔄 מעדכן את `{config_name}` לערך `{message_text}`...")

        # קריאה לפונקציה ששומרת את ההגדרה החדשה
        success = Set_Config_InServer(config_name, message_text)

        if success:
            await event.respond(f"✅ **ההגדרה `{config_name}` עודכנה בהצלחה!**")
        else:
            await event.respond(f"❌ **עדכון ההגדרה `{config_name}` נכשל.**")

# 📌 מאזין ללחיצות על כפתורים
@client.on(events.CallbackQuery)
async def callback_handler(event):
    data = event.data.decode("utf-8")

    if not is_authorized(event.sender_id):
        await event.answer("⛔ עליך להזין סיסמה תחילה.", alert=True)
        return

    if data == "run_bot":
        if Run_MTScanner():
            await event.edit("✅ **הבוט רץ בהצלחה!**")
            senderDetails = await event.get_sender()
            user_Name = senderDetails.username if senderDetails.username else event.sender_id

            await client.send_message(channel_id,
            f"<b>✅ הבוט הורץ!</b>\n"
            f"👤 <b>משתמש:</b> {user_Name}\n",parse_mode="html")

        else:
            await event.edit("❌ **כשל בשליחת הקריאה**")
        await show_main_menu(event)

    elif data == "stop_bot":
        if Stop_MTScanner():
            await event.edit("🛑 **הבוט נעצר!**")
            senderDetails = await event.get_sender()
            user_Name = senderDetails.username if senderDetails.username else event.sender_id

            await client.send_message(channel_id,
            f"<b>🛑 הבוט נעצר!</b>\n"
            f"👤 <b>משתמש:</b> {user_Name}\n",parse_mode="html")

        else:
            await event.edit("❌ **כשל בשליחת הקריאה**")
        await show_main_menu(event)

    elif data == "status_bot":
        currStatus = Get_MTScanner_Status().get(HEADER_STATUS_CURRENTSTATUS)
        if currStatus == HEADER_STATUS_VALUE_RUNNING:
            await event.edit("🔄 **סטטוס הבוט:** רץ!")
        elif currStatus == HEADER_STATUS_VALUE_STOPPED:
            await event.edit("🛑 **הבוט לא פעיל!**")
        elif currStatus == HEADER_STATUS_VALUE_LOADINGDATA:
            await event.edit("⏳ **הבוט טוען נתונים... אנא המתן.**")
        else:
            await event.edit("❌ **המידע לא זמין כרגע. נסה שוב מאוחר יותר.**")
        await show_main_menu(event)

    elif data == "settings":
        await settings_bot(event)
    elif data == "main_menu":
        await show_main_menu(event)
    elif data == "set_settings":
        user_states[event.sender_id] = "awaiting_config_selection"
        buttons = [[Button.inline(config, f"config_set_{config}")] for config in All_Configs_names]
        buttons.append([Button.inline("🔙 חזרה", b"settings")])
        await event.respond("✏️ **בחר הגדרה כדי לערוך אותה:**", buttons=buttons)

    elif data == "get_settings":
        user_states[event.sender_id] = "awaiting_config_selection"

        # יצירת כפתורים עם שמות ההגדרות
        buttons = [[Button.inline(config, f"config_get_{config}")] for config in All_Configs_names]

        # כפתור חזרה
        buttons.append([Button.inline("🔙 חזרה", b"settings")])

        await event.respond("📜 **בחר הגדרה להצגת הערך שלה:**", buttons=buttons)

    elif data.startswith("config_set_"):
        config_name = data[len("config_set_"):]
        user_states[event.sender_id] = f"awaiting_config_value_{config_name}"
        await event.respond(f"✏️ **הזן ערך חדש עבור ההגדרה:** `{config_name}`")

    elif data.startswith("config_get_"):  # כאשר המשתמש בוחר הגדרה
        config_name = data[len("config_get_"):]  # חילוץ שם ההגדרה

        await event.edit(f"🔄 מחפש את ההגדרה: **{config_name}**...")

        # קריאה לשרת לקבלת המידע
        config_data = Get_Config_FromServer(config_name)

        if config_data:
            config_value = config_data.get("ConfigValue", "לא נמצא")
            body = config_data.get("ConfigExplain", "אין מידע נוסף")

            response_message = (
                f"✅ **תוצאה עבור {config_name}:**\n"
                f"📌 **שם ההגדרה:** {config_name}\n"
                f"📌 **ערך ההגדרה:** {config_value}\n\n"
                f"📄 **מידע נוסף:**\n\n{body}\n"
            )
        else:
            response_message = "❌ שגיאה: לא ניתן היה לקבל את המידע מהשרת."

        await event.respond(response_message, parse_mode="html")
        await show_main_menu(event)

    else:
        await event.edit("❌ פיצ'ר זה תחת פיתוח, יתווסף בקרוב.")

# 📌 הפעלת הבוט
with client:
    print("✅ הבוט מאזין להודעות... לחץ Ctrl+C כדי לעצור.")
    client.run_until_disconnected()
