# this file contains the class that handles sending sms messages
from twilio.rest import Client


class TextSMS:
    account_sid = 'ENVIRONMENT VARIABLE'
    auth_token = 'ENVIRONMENT VARIABLE'

    # def __init__(self, to_num, from_num):
    #     self.client = Client(self.account_sid, self.auth_token)
    #     self.to_num = to_num
    #     self.from_num = from_num

    @staticmethod
    def send_message(self, message, from_num, to_num):
        message = self.client.messages \
            .create(
            body=message,
            from_=from_num,
            to=to_num
        )