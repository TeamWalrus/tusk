export type CardType = "hid" | "gallagher" | "unknown";

export interface Card {
  card_type: CardType;
  bit_length: number;
  facility_code: number;
  card_number: number;
  region_code?: number;
  issue_level?: number;
  hex: string;
  raw: string;
}

export interface DeviceSettings {
  capturing: boolean;
  debug_mode: boolean;
  version: string;
}

export interface WiFiConfig {
  ssid: string;
  password: string;
  channel: string;
  hidessid: string;
}

export interface FilesystemInfo {
  totalBytes: number;
  usedBytes: number;
}
