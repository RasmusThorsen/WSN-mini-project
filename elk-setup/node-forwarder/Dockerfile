FROM node:12.16.2-alpine3.11 AS base
WORKDIR /app
COPY . .
RUN npm install
RUN apk --no-cache add curl
ENTRYPOINT [ "npm", "start" ]