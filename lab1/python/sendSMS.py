# this file contains the class that handles sending sms messages
from twilio.rest import Client
import os


class TextSMS:
    auth_token = os.environ['AUTH_TOKEN']
    account_sid = os.environ['ACCOUNT_SID']
    # client = Client(account_sid, auth_token)

    # def __init__(self, to_num, from_num):
    #     self.client = Client(self.account_sid, self.auth_token)
    #     self.to_num = to_num
    #     self.from_num = from_num

    @staticmethod
    def send_message(message, from_num, to_num):
        client = Client(TextSMS.account_sid, TextSMS.auth_token)
        message = client.messages \
            .create(
            body=message,
            from_=from_num,
            to=to_num
        )
