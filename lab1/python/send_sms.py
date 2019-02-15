# this file contains the class that handles sending sms messages
from twilio.rest import Client


class TextSMS:
    account_sid = 'ENVIRONMENT VARIABLE'
    auth_token = 'ENVIRONMENT VARIABLE'

    def __init__(self, to_num, from_num):
        self.client = Client(self.account_sid, self.auth_token)
        self.to_num = to_num
        self.from_num = from_num

    def send_message(self, message):
        message = self.client.messages \
            .create(
            body=message,
            from_= self.from_num,
            to=self.to_num
        )
