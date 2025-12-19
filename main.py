import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders

# Настройка SMTP
server = smtplib.SMTP('localhost', 25)  # Адрес локального SMTP сервера без TLS

# Авторизация
server.login("user123@localdomain.com", "12345678")

# Создание сообщения
msg = MIMEMultipart()
msg['From'] = "user123@localdomain.com"
msg['To'] = "user123@localdomain.com"
msg['Subject'] = "Test Email with Attachment"

# Тело письма
body = "This is a test email with attachment"
msg.attach(MIMEText(body, 'plain'))

# Вложение
filename = "file.txt"  # Путь к файлу
attachment = open(filename, "rb")

part = MIMEBase('application', 'octet-stream')
part.set_payload(attachment.read())
encoders.encode_base64(part)
part.add_header('Content-Disposition', f"attachment; filename= {filename}")
msg.attach(part)

# Отправка письма
server.sendmail("user123@localdomain.com", "user123@localdomain.com", msg.as_string())

# Завершение работы
server.quit()
